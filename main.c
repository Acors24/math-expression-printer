#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>

#include "ExprTree.h"

typedef struct region
{
    int left, right, top, bottom;
} Region;

ExprTree gtree;
GtkWidget *entry;
GtkWidget *area;
GtkWidget *gwindow;
cairo_surface_t *surface = NULL;

int get_char_width(int font_size)
{
    return font_size * 2.25/3;
}

Region create_region(int left, int right, int top, int bottom)
{
    Region region;
    region.left = left;
    region.right = right;
    region.top = top;
    region.bottom = bottom;
    return region;
}

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);

    return FALSE;
}

void draw_par(cairo_t *cr, Region region)
{
    const int width = region.right - region.left;
    const int height = region.bottom - region.top;
    const double radius = sqrt(width * width + height * height) * 1.5;
    const double a = atan((height / 2) / radius);
    const int offset = 8;
    cairo_move_to(cr, region.left, region.bottom - offset);
    cairo_arc(cr, region.left + radius - radius / 50, (region.top + region.bottom) / 2 - offset, radius, 3.1415 - a, 3.1415 + a);
    cairo_move_to(cr, region.right, region.top - offset);
    cairo_arc(cr, region.right - radius + radius / 50, (region.top + region.bottom) / 2 - offset, radius, -a, a);
    cairo_stroke(cr);
}

void draw_expression(cairo_t *cr, ExprTree tree, Region region, int font_size, bool par, bool center)
{
    if (tree == NULL)
        return;

    if (par)
        draw_par(cr, region);

    cairo_set_font_size(cr, font_size);
    if (tree->isOperator)
    {
        int gap = (region.right - region.left - get_width(tree) * get_char_width(font_size)) / 2;
        if (tree->operator == '/')
        {
            int y = (region.top + region.bottom) / 2 - font_size * 3.5/10;
            cairo_rectangle(cr, region.left + gap, y, region.right - region.left - gap * 2, 1);
            draw_expression(cr, tree->l, create_region(
                region.left + gap,
                region.right - gap,
                region.top,
                y
            ), font_size / 1.15, false, true);
            draw_expression(cr, tree->r, create_region(
                region.left + gap,
                region.right - gap,
                y + font_size,
                region.bottom
            ), font_size / 1.15, false, true);
            cairo_set_font_size(cr, font_size);
        }
        else if (tree->operator == '^')
        {
            int width_l = get_width(tree->l) * get_char_width(font_size);
            int x = region.left + width_l + gap;
            int y = (region.top + region.bottom) / 2 - font_size * 3.5/10;
            draw_expression(cr, tree->l, create_region(
                region.left + gap,
                x,
                region.top,
                region.bottom
            ), font_size, tree->l->isOperator, true);
            draw_expression(cr, tree->r, create_region(
                x,
                region.right - gap,
                region.top - font_size * 2.0/10,
                y
            ), font_size / 1.2, false, false);
        }
        else
        {
            int x = region.left + get_width(tree->l) * get_char_width(font_size) + gap;
            int height_l = get_height(tree->l) * font_size;
            int height_r = get_height(tree->r) * font_size;
            cairo_move_to(cr, x, (region.top + region.bottom) / 2 + (tree->operator == '*' ? font_size / 5 : 0));
            char text[1] = {tree->operator};
            cairo_show_text(cr, text);
            bool par_l = (tree->operator == '*' && tree->l->isOperator && in(tree->l->operator, "+-"));
            bool par_r = (in(tree->operator, "-*") && tree->r->isOperator && in(tree->r->operator, "+-"));
            draw_expression(cr, tree->l, create_region(
                region.left + gap,
                x,
                region.top + (region.bottom - region.top - height_l) / 2,
                region.top + (region.bottom - region.top - height_l) / 2 + height_l
            ), font_size, par_l, true);
            draw_expression(cr, tree->r, create_region(
                x + get_char_width(font_size),
                region.right - gap,
                region.top + (region.bottom - region.top - height_r) / 2,
                region.top + (region.bottom - region.top - height_r) / 2 + height_r
            ), font_size, par_r, true);
        }
    }
    else
    {
        int len = strlen(tree->operand), char_width = get_char_width(font_size);
        int x = ((region.right - region.left) - (len * char_width)) / 2;
        cairo_move_to(cr, region.left + x * center, (region.top + region.bottom) / 2);
        cairo_show_text(cr, tree->operand);
    }

    // Uncomment to see space allocated for each part of the expression
    /*
    cairo_set_source_rgba(cr, 1, 0, 0, 0.2);
    cairo_rectangle(cr, region.left, region.top, region.right - region.left, region.bottom - region.top);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    */
}

void draw_full_expression(GtkWidget *widget)
{
    cairo_t *cr;
    cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    int font_size = 25;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 10, font_size);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    int width = get_width(gtree) * get_char_width(font_size);
    int height = get_height(gtree) * font_size;
    draw_expression(cr, gtree, create_region(
        (allocation.width - width) / 2,
        (allocation.width - width) / 2 + width,
        (allocation.height - height) / 2,
        (allocation.height - height) / 2 + height
    ), font_size, false, true);

    cairo_fill(cr);
    cairo_destroy(cr);

    GdkRectangle update_rect;
    update_rect.x = 0;
    update_rect.y = 0;
    update_rect.width = allocation.width;
    update_rect.height = allocation.height;
    gdk_window_invalidate_rect(gtk_widget_get_window(widget), &update_rect, FALSE);
}

gboolean configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    GtkAllocation allocation;
    cairo_t *cr;

    if (surface)
        cairo_surface_destroy(surface);

    gtk_widget_get_allocation(widget, &allocation);
    surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR,
                                                allocation.width, allocation.height);

    cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    draw_full_expression(area);

    cairo_destroy(cr);

    return TRUE;
}

gboolean parse_button_event(GtkWidget *widget, GtkWidget *data)
{
    if (surface == NULL)
        return FALSE;

    const char* expr = gtk_entry_get_text(GTK_ENTRY(entry));
    free_tree(gtree);
    int error_code = 0;
    gtree = parse(expr, &error_code);
    if (gtree == NULL)
    {
        char msg[50];
        switch (error_code)
        {
            case 1:
                strcpy(msg, "Missing \")\".");
                break;
            case 2:
                strcpy(msg, "Redundant \")\".");
                break;
            default:
                strcpy(msg, "Unknown error.");
        }

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gwindow),
                                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Error");
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", msg);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return FALSE;
    }

    draw_full_expression(data);

    return TRUE;
}

void activate(GtkApplication *app, gpointer user_data)
{
    //GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *button;
    GtkWidget *hsep;
    //GtkWidget *area;

    // WINDOW
    gwindow = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(gwindow), "Expression printer");
    gtk_window_set_default_size(GTK_WINDOW(gwindow), 500, 250);

    // VBOX
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_container_add(GTK_CONTAINER(gwindow), vbox);

    // HBOX
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_valign(hbox, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // HSEPARATOR
    hsep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), hsep, FALSE, FALSE, 0);

    // DRAWING AREA
    area = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(area), "draw", G_CALLBACK(draw), NULL);
    g_signal_connect(G_OBJECT(area), "configure-event", G_CALLBACK(configure_event), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), area, TRUE, TRUE, 0);

    // ENTRY
    entry = gtk_entry_new();
    //gtk_entry_set_text(GTK_ENTRY(entry), "y=z-1=(100x-1*((17)+(2/x^3))/31*80x^2)");
    //gtk_entry_set_text(GTK_ENTRY(entry), "y=(x-sin(x))/x^3");
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(parse_button_event), area);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

    // BUTTON
    button = gtk_button_new_with_label("Print");
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(parse_button_event), area);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

    gtk_widget_show_all(gwindow);
    gtk_window_present(GTK_WINDOW(gwindow));
}

int main(int argc, char* argv[])
{
    GtkApplication *app;
    int status = 0;

    app = gtk_application_new("i329827.expressionprinter", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
