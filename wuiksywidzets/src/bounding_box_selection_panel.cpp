#include <bounding_box_selection_panel.hpp>
#include <wx/image.h>

BEGIN_EVENT_TABLE(wxImagePanel, wxPanel)

EVT_LEFT_DOWN(wxImagePanel::mouseDown)
EVT_KEY_DOWN(wxImagePanel::keyPressed)
EVT_MOUSEWHEEL(wxImagePanel::mouseWheelMoved)

EVT_PAINT(wxImagePanel::paintEvent)

END_EVENT_TABLE()

void
wxImagePanel::mouseDown(wxMouseEvent& event)
{
  auto point = event.GetLogicalPosition(wxClientDC(this));
  wxPoint map_point(point.x / scale_, point.y / scale_);
  wxPoint world_point(map_point.x * 32 + 32768 * tile_x_, map_point.y * 32 + 32768 * tile_y_);
  temp_points_.emplace_back(std::pair<int, int>{world_point.x, world_point.y});
  this->paintNow();
}

void
wxImagePanel::mouseWheelMoved(wxMouseEvent& event)
{
  scale_ += event.GetWheelRotation() * 0.0005f;
  if (scale_ > 10.0f)
    scale_ = 10.0f;
  if (scale_ < 0.3f)
    scale_ = 0.3f;
  this->paintNow();
}

void
wxImagePanel::keyPressed(wxKeyEvent& event)
{
}

void
wxImagePanel::on_update(wxCommandEvent& event)
{
  box_.box_points = temp_points_;
}

void
wxImagePanel::on_delete(wxCommandEvent& event)
{
  temp_points_.clear();
  this->paintNow();
}

wxImagePanel::wxImagePanel(wxWindow* parent,
                           wxString file,
                           wxBitmapType format,
                           bounding_box& box,
                           int tile_x,
                           int tile_y)
  : wxPanel(parent)
  , scale_(1.0f)
  , box_(box)
  , tile_x_(tile_x - 20) // these subtracted vals are lineage constants for 0,0 point
  , tile_y_(tile_y - 18)
  , temp_points_(box.box_points)
{
  // load the file... ideally add a check to see if loading was successful
  image.LoadFile(file, format);
  wxSize original_image_size = image.GetSize();
  wxLogMessage("Loaded map dimensions %dx%d", original_image_size.x, original_image_size.y);
  if (original_image_size.x != original_image_size.y)
    wxLogMessage("Fuck, I don't support non-rectangular maps yet xD Hope noone tried");
  scale_to_1024_ = 1024.0f / (float)original_image_size.x;
  wxButton* update_button =
    new wxButton(this, wxID_ANY, "Update box", wxPoint(0, 0), wxSize(90, 30));
  wxButton* delete_button =
    new wxButton(this, wxID_ANY, "Delete box", wxPoint(90, 0), wxSize(90, 30));

  update_button->Bind(wxEVT_BUTTON, &wxImagePanel::on_update, this);
  delete_button->Bind(wxEVT_BUTTON, &wxImagePanel::on_delete, this);
}

void
wxImagePanel::paintEvent(wxPaintEvent& evt)
{
  wxPaintDC dc(this);
  render(dc);
}

void
wxImagePanel::paintNow()
{
  wxClientDC dc(this);
  render(dc);
}

void
wxImagePanel::render(wxDC& dc)
{
  dc.Clear();
  dc.SetUserScale(scale_ * scale_to_1024_, scale_ * scale_to_1024_);
  dc.DrawBitmap(image, 0, 0, false);

  wxPointList pl; // kurwa jak tego uzyc?
  int count = 0;
  wxPoint pts[100];
  for (int i = 0; i < 100 && i < temp_points_.size(); ++i, ++count)
  {
    pts[i].x = (temp_points_[i].first - 32768 * tile_x_) / 32; // TOO TU JESZCZE SKALA
    pts[i].y = (temp_points_[i].second - 32768 * tile_y_) / 32;
  }

  wxPen mypen(*wxCYAN, 5);
  dc.SetPen(mypen);
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.DrawPolygon(count, pts, 0, 0, wxODDEVEN_RULE);
}
