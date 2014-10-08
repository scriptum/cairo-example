#include <cairo.h>
#include <gtk/gtk.h>

static void draw_gradient1(cairo_t *);
static void draw_gradient2(cairo_t *);
static void draw_gradient3(cairo_t *);

static gboolean
on_expose_event(GtkWidget *widget,
		GdkEventExpose *event G_GNUC_UNUSED,
		gpointer data G_GNUC_UNUSED)
{
	cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));

	draw_gradient1(cr);
	draw_gradient2(cr);
	draw_gradient3(cr);

	cairo_destroy(cr);

	return FALSE;
}

static void
draw_gradient1(cairo_t *cr)
{
	cairo_pattern_t *pat1;  
	gdouble j;
	gint count = 1;

	pat1 = cairo_pattern_create_linear(0.0, 0.0,  350.0, 350.0);

	for(j = 0.1; j < 1; j += 0.1)
	{
		if(count % 2)
		{
			cairo_pattern_add_color_stop_rgb(pat1, j, 0, 0, 0);
		}
		else
		{ 
			cairo_pattern_add_color_stop_rgb(pat1, j, 1, 0, 0);
		}
		count++;
	}

	cairo_rectangle(cr, 20, 20, 300, 100);
	cairo_set_source(cr, pat1);
	cairo_fill(cr);  
	
	cairo_pattern_destroy(pat1);
}

static void
draw_gradient2(cairo_t *cr)
{
	cairo_pattern_t *pat2;
	gdouble i;
	gint count = 1;

	pat2 = cairo_pattern_create_linear(0.0, 0.0,  350.0, 0.0);

	for(i = 0.05; i < 0.95; i += 0.025)
	{
		if(count % 2)
		{
			cairo_pattern_add_color_stop_rgb(pat2, i, 0, 0, 0);
		}
		else
		{ 
			cairo_pattern_add_color_stop_rgb(pat2, i, 0, 0, 1);
		}
		count++;
	}

	cairo_rectangle(cr, 20, 140, 300, 100);
	cairo_set_source(cr, pat2);
	cairo_fill(cr);  
	
	cairo_pattern_destroy(pat2);
}

static void
draw_gradient3(cairo_t *cr)
{
	cairo_pattern_t *pat3;
	pat3 = cairo_pattern_create_linear(20.0, 260.0, 20.0, 360.0);

	cairo_pattern_add_color_stop_rgb(pat3, 0.1, 0, 0, 0);
	cairo_pattern_add_color_stop_rgb(pat3, 0.5, 1, 1, 0);
	cairo_pattern_add_color_stop_rgb(pat3, 0.9, 0, 0, 0);

	cairo_rectangle(cr, 20, 260, 300, 100);
	cairo_set_source(cr, pat3);
	cairo_fill(cr);  

	cairo_pattern_destroy(pat3);
}

int
main(int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *darea;  

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER (window), darea);

	g_signal_connect(G_OBJECT(darea), "expose-event", 
			G_CALLBACK(on_expose_event), NULL);  
	g_signal_connect(G_OBJECT(window), "destroy",
			G_CALLBACK(gtk_main_quit), NULL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 340, 390); 
	gtk_window_set_title(GTK_WINDOW(window), "Linear gradients");

	gtk_widget_set_app_paintable(window, TRUE);
	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
