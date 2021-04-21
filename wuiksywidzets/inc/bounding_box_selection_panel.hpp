#pragma once

#include <bot_config.hpp>
#include <wx/sizer.h>
#include <wx/wx.h>

class wxImagePanel : public wxPanel
{
  wxBitmap image;
  float scale_;
  bounding_box& box_;
  int tile_x_;
  int tile_y_;
  std::vector<std::pair<int, int>> temp_points_;
  float scale_to_1024_; // hotfix cause not every map is 1024x1024 and it fucks up the bounding box

public:
  wxImagePanel(wxWindow* parent,
               wxString file,
               wxBitmapType format,
               bounding_box& box,
               int tile_x,
               int tile_y);

  void paintEvent(wxPaintEvent& evt);
  void paintNow();

  void render(wxDC& dc);

  // some useful events

  void mouseMoved(wxMouseEvent& event);
  void mouseDown(wxMouseEvent& event);
  void mouseWheelMoved(wxMouseEvent& event);
  void mouseReleased(wxMouseEvent& event);
  void rightClick(wxMouseEvent& event);
  void mouseLeftWindow(wxMouseEvent& event);
  void keyPressed(wxKeyEvent& event);
  void keyReleased(wxKeyEvent& event);

  void on_update(wxCommandEvent& event);
  void on_delete(wxCommandEvent& event);

  DECLARE_EVENT_TABLE()
};
