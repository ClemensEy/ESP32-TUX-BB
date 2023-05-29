/*
MIT License

Copyright (c) 2022 Sukesh Ashok Kumar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
 * @file tux_panel.c
 *
 */

 /*********************
  *      INCLUDES
  *********************/
#include "tux_panel.h"
#if TUX_USE_PANEL

  /*********************
   *      DEFINES
   *********************/
#define MY_CLASS    &tux_panel_class

   /**********************
    *      TYPEDEFS
    **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t tux_panel_class = {
    .base_class = &lv_obj_class,
    .width_def = LV_PCT(100), // LV_DPI_DEF * 2,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(tux_panel_t)
};

const lv_obj_class_t tux_panel_content_class = {
    .base_class = &lv_obj_class,
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(lv_obj_t)
};

 /**********************
  *   GLOBAL FUNCTIONS
  **********************/

void default_panel_styles()
{
    //lv_theme_t * current_theme = 
}

lv_obj_t * tux_panel_create(lv_obj_t* parent, const char* title, lv_coord_t height)
{
    LV_LOG_INFO("begin"); // Log an informational message ("begin")

    // Create a new object of type "tux_panel_class" and initialize it
    lv_obj_t * obj = lv_obj_class_create_obj(&tux_panel_class,parent);
    lv_obj_class_init_obj(obj);

    // Cast the object as a "tux_panel_t" type
    tux_panel_t * _panel = (tux_panel_t *)obj;

    LV_ASSERT_MALLOC(obj); // Assert that the memory allocation for "obj" was successful
    if (obj == NULL) return NULL; // If the allocation failed, return NULL

    // Set the height of the object to the specified "height" value
    lv_obj_set_height(obj,height);

    // Set the flex flow of the object to "LV_FLEX_FLOW_ROW_WRAP"
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP);
    
    bool has_title = title && strlen(title) > 0; // Check if the "title" is not empty
    if (has_title) {

        // Create a title panel as a child of "obj"
        _panel->title_panel = lv_obj_create(obj);
        lv_obj_set_style_pad_hor(_panel->title_panel,5,0); // Set horizontal padding
        lv_obj_set_style_pad_ver(_panel->title_panel,2,0); // Set vertical padding
        lv_obj_set_style_radius(_panel->title_panel,3,0); // Set border radius
        lv_obj_set_size(_panel->title_panel,LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set size
        //lv_obj_set_x(_panel->title_panel,100); // Set the x-position of the title panel

        // Create a title label as a child of the title panel
        _panel->title = lv_label_create(_panel->title_panel);
        lv_label_set_text(_panel->title, title); // Set the text of the title label
        //lv_label_set_long_mode(_panel->title, LV_LABEL_LONG_SCROLL_CIRCULAR); // Set long mode
        lv_obj_set_width(_panel->title, LV_SIZE_CONTENT); // Set width
        lv_obj_set_style_pad_all(_panel->title, 3,0); // Set padding
        lv_obj_set_style_align(_panel->title,LV_ALIGN_TOP_LEFT,0); // Set alignment

        // Add an event callback for later use (not implemented in the provided code)
        // lv_obj_t * icon_edit = lv_label_create(_panel->title_panel);
        // lv_obj_set_style_align(icon_edit,LV_ALIGN_TOP_RIGHT,0);
        // lv_label_set_text(icon_edit, LV_SYMBOL_SETTINGS);
    }

    // Create the content object as a child of "obj"
    _panel->content = lv_obj_class_create_obj(&tux_panel_content_class, obj);
    lv_obj_set_style_pad_all(_panel->content,5,0); // Set padding for the content object

    return obj; // Return the created object
}
lv_obj_t * chart_panel_create(lv_obj_t* parent, const char* title, lv_coord_t height)
{
    LV_LOG_INFO("begin"); // Log an informational message ("begin")

    // Create a new object of type "tux_panel_class" and initialize it
    lv_obj_t * obj = lv_obj_class_create_obj(&tux_panel_class,parent);
    lv_obj_class_init_obj(obj);

    // Cast the object as a "tux_panel_t" type
    tux_panel_t * _panel = (tux_panel_t *)obj;

    LV_ASSERT_MALLOC(obj); // Assert that the memory allocation for "obj" was successful
    if (obj == NULL) return NULL; // If the allocation failed, return NULL

    // Set the height of the object to the specified "height" value
    lv_obj_set_height(obj,height);

    // Set the flex flow of the object to "LV_FLEX_FLOW_ROW_WRAP"
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP);
    
    bool has_title = title && strlen(title) > 0; // Check if the "title" is not empty
    if (has_title) {

        // Create a title panel as a child of "obj"
        _panel->title_panel = lv_obj_create(obj);
        lv_obj_set_style_pad_hor(_panel->title_panel,0,0); // Set horizontal padding
        lv_obj_set_style_pad_ver(_panel->title_panel,0,0); // Set vertical padding
        lv_obj_set_style_radius(_panel->title_panel,0,0); // Set border radius
        lv_obj_set_size(_panel->title_panel,LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set size
        //lv_obj_set_x(_panel->title_panel,100); // Set the x-position of the title panel

        // Create a title label as a child of the title panel
        _panel->title = lv_label_create(_panel->title_panel);
        lv_label_set_text(_panel->title, title); // Set the text of the title label
        //lv_label_set_long_mode(_panel->title, LV_LABEL_LONG_SCROLL_CIRCULAR); // Set long mode
        lv_obj_set_width(_panel->title, LV_SIZE_CONTENT); // Set width
        lv_obj_set_style_pad_all(_panel->title, 3,0); // Set padding
        lv_obj_set_style_align(_panel->title,LV_ALIGN_TOP_LEFT,0); // Set alignment

        // Add an event callback for later use (not implemented in the provided code)
        // lv_obj_t * icon_edit = lv_label_create(_panel->title_panel);
        // lv_obj_set_style_align(icon_edit,LV_ALIGN_TOP_RIGHT,0);
        // lv_label_set_text(icon_edit, LV_SYMBOL_SETTINGS);
    }

    // Create the content object as a child of "obj"
    _panel->content = lv_obj_class_create_obj(&tux_panel_content_class, obj);
    lv_obj_set_style_pad_all(_panel->content,0,0); // Set padding for the content object

    return obj; // Return the created object
}

void tux_panel_set_height(lv_obj_t* obj, lv_coord_t value)
{
    lv_obj_set_height(obj, value);
}

void tux_panel_set_title_color(lv_obj_t* obj,lv_color_t value)
{
    tux_panel_t* _panel = (tux_panel_t*)obj;
    lv_obj_set_style_text_color(_panel->title,value,0);
}

void tux_panel_set_title_bg_color(lv_obj_t* obj,lv_color_t value)
{
    tux_panel_t* _panel = (tux_panel_t*)obj;
    lv_obj_set_style_bg_color(_panel->title_panel,value,0);
}

void tux_panel_set_content_bg_color(lv_obj_t* obj,lv_color_t value)
{
    tux_panel_t* _panel = (tux_panel_t*)obj;
    lv_obj_set_style_bg_color(_panel->content,value,0);
}

lv_obj_t * tux_panel_get_title(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    tux_panel_t * _panel = (tux_panel_t *)obj;
    return _panel->title;
}

lv_obj_t* tux_panel_get_content(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    tux_panel_t * _panel = (tux_panel_t *)obj;
    return _panel->content;
}

lv_obj_t* tux_panel_get_title_panel(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    tux_panel_t * _panel = (tux_panel_t *)obj;
    return _panel->title_panel;
}
#endif /*LV_USE_PANEL*/
