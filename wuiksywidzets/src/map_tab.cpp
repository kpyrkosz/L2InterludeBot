#include <event_identifiers.hpp>
#include <map_tab.hpp>

// test zone
class ScrolledImageComponent : public wxScrolledWindow
{
  wxBitmap* bitmap;
  int w, h;

public:
  ScrolledImageComponent(wxWindow* parent, wxWindowID id, wxString image_path)
    : wxScrolledWindow(parent, id)
  {
    wxImage image(image_path);
    if (!image.IsOk())
    {
      wxMessageBox(wxT("there was an error loading the image"));
      return;
    }

    w = image.GetWidth();
    h = image.GetHeight();

    /* init scrolled area size, scrolling speed, etc. */
    SetScrollbars(1, 1, w, h, 0, 0);

    bitmap = new wxBitmap(image);
  }
  ~ScrolledImageComponent() { delete bitmap; }
  void OnDraw(wxDC& dc)
  {
    /* render the image - in a real app, if your scrolled area
       is somewhat big, you will want to draw only visible parts,
       not everything like below */
    dc.DrawBitmap(*bitmap, 0, 0, false);

    // also check wxScrolledWindow::PrepareDC
  }
};
// po
map_tab::map_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
{
  /*bounding_box box;
  auto* qq = new wxImagePanel(
    my_parent,
    wxString::Format(R"(C:\Users\EmKa\Downloads\Maps (Interlude - H5)\Maps\%d_%d.jpg)", 20, 20),
    wxBITMAP_TYPE_JPEG,
    box,
    20,
    20);*/

  // content_panel->SetSizer(content_sizer);
  // my_parent_->AddPage(qq, "Out of party", false, 12);

  ScrolledImageComponent* my_image = new ScrolledImageComponent(
    my_parent, wxID_ANY, wxString::Format(R"(rsrc\Maps\%d_%d.jpg)", 20, 20));

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(my_image, 1, wxALL | wxEXPAND, 120);
  // frame->SetSizer(sizer);
  my_parent_->AddPage(my_image, "Map", false, 14);
}

void
map_tab::restore_most_recent_config()
{
}

void
map_tab::save_configs(wxFileConfig& config_to_write)
{
}

void
map_tab::load_configs(wxFileConfig& config_to_load)
{
}

void
map_tab::update_and_serialize(binary_serializer& bs)
{
}
