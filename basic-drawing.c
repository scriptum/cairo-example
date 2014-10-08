#include <cairo.h>
#include <gtk/gtk.h>

/* Left click - drawing, right click - clear */

#define MAX_LINES 1024

static void do_drawing(cairo_t *);

struct {
	gint count;
	guint16 x[MAX_LINES];
	guint16 y[MAX_LINES];
} glob;

static gboolean
on_expose_event(GtkWidget *widget,
		GdkEventExpose *event G_GNUC_UNUSED,
		gpointer data G_GNUC_UNUSED)
{
	cairo_t *cr = gdk_cairo_create(widget->window);
	do_drawing(cr);
	cairo_destroy(cr);

	return FALSE;
}

static void
do_drawing(cairo_t *cr)
{
	gint i;

	cairo_set_source_rgb(cr, 0., 0., 0.);
	cairo_set_line_width(cr, 0.5);

	cairo_move_to(cr, glob.x[0], glob.y[0]);
	for(i = 1; i < glob.count; i++)
	{
		cairo_line_to(cr, glob.x[i], glob.y[i]);
	}

	cairo_stroke(cr);    
}

static gboolean
clicked(GtkWidget *widget, GdkEventButton *event, gpointer data G_GNUC_UNUSED)
{
	switch(event->button)
	{
		case 1:
			if(glob.count >= MAX_LINES)
			{
				glob.count--;
			}
			glob.x[glob.count] = event->x;
			glob.y[glob.count] = event->y;
			glob.count++;
			break;
		case 3:
			glob.count = 0;
			break;
	}
	gtk_widget_queue_draw(widget);
	return TRUE;
}

int
main(int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *darea;
	
	glob.count = 0;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), darea);
 
	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);

	g_signal_connect(G_OBJECT(darea), "expose-event", 
			G_CALLBACK(on_expose_event), NULL); 
	g_signal_connect(window, "destroy",
			G_CALLBACK(gtk_main_quit), NULL);  
		
	g_signal_connect(window, "button-press-event", 
			G_CALLBACK(clicked), NULL);
 
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300); 
	gtk_window_set_title(GTK_WINDOW(window), "Lines");

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
