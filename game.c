#include <cairo.h>
#include <gtk/gtk.h>
/* для обработки нажатия клавиш */
#include <gdk/gdkkeysyms.h>
/* для математики */
#include <math.h>

/* размер экрана */
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

/* кадров в секунду */
#define FPS 60

/* радиус игрока */
#define PLAYER_RADIUS 20.0

/* радиус пуль */
#define BULLET_RADIUS 2.0
#define BULLET_SPEED 4.0

/* скорость перемещения */
#define ANGULAR_STEP 0.05
#define SPEED_STEP 2.0

typedef struct vec {
	gdouble x, y;
} vec;

struct {
	vec position;
	vec speed;
	gdouble angle;
	gdouble angular_speed;
	guint health;
	guint score;
} player = {
	.position = {
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2
	}, 
	.health = 100, 
	.score = 0
};

struct {
	vec position;
	vec speed;
	gboolean alive;
} bullet = {.alive = FALSE};

/* обработка нажатий клавиш */
guint keystate = 0;
enum {
	KEY_W     = 1 << 0,
	KEY_S     = 1 << 1,
	KEY_A     = 1 << 2,
	KEY_D     = 1 << 3,
	KEY_LEFT  = 1 << 4,
	KEY_RIGHT = 1 << 5,
};

static void
draw_player(cairo_t *cr)
{
	/* http://zetcode.com/gfx/cairo/transformations/
	 * cairo_save/cairo_restore изолируют наши пляски с трансформациями */
	cairo_save(cr);
	cairo_translate(cr, player.position.x, player.position.y);
	cairo_rotate(cr, player.angle);
	/* нос */
	cairo_move_to(cr, -8.0, -18.0);
	cairo_line_to(cr, 0.0, -40.0);
	cairo_line_to(cr, 8.0, -18.0);
	cairo_stroke(cr);
	/* глаза */
	#define EYE 0.9
	cairo_arc(cr, -16.0, -12.0, 6., M_PI - EYE, -EYE);
	cairo_stroke(cr);
	cairo_arc(cr, 16.0, -12.0, 6., M_PI + EYE, EYE);
	cairo_stroke(cr);
	/* тело колобка */
	cairo_arc(cr, 0.0, 0.0, PLAYER_RADIUS, 0., 2 * M_PI);
	cairo_stroke(cr);
	cairo_restore(cr);
}

static void
draw_bullet(cairo_t *cr)
{
	if(FALSE == bullet.alive)
	{
		return;
	}
	cairo_arc(cr, bullet.position.x, bullet.position.y, BULLET_RADIUS, 0., 2 * M_PI);
	cairo_stroke(cr);
}

/* Рисование мира */
static gboolean
draw_world(GtkWidget *widget G_GNUC_UNUSED,
		GdkEventExpose *event G_GNUC_UNUSED,
		gpointer data G_GNUC_UNUSED)
{
	cairo_t *cr;
	static gchar buffer[32];

	cr = gdk_cairo_create(widget->window);
	draw_player(cr);
	draw_bullet(cr);
	cairo_move_to(cr, 5, 20);
	cairo_set_font_size(cr, 14);
	snprintf(buffer, sizeof(buffer), "Score: %d", player.score);
	cairo_show_text(cr, buffer);

	cairo_destroy(cr);

	return FALSE;
}

static void
bullet_shot()
{
	if(FALSE == bullet.alive)
	{
		#define DELTA (M_PI_2 + M_PI_4)
		bullet.speed.x = BULLET_SPEED * (cos(player.angle - DELTA) - sin(player.angle - DELTA)) + player.speed.x;
		bullet.speed.y = BULLET_SPEED * (sin(player.angle - DELTA) + cos(player.angle - DELTA)) + player.speed.y;
		bullet.position.x = player.position.x;
		bullet.position.y = player.position.y;
		bullet.alive = TRUE;
		#undef DELTA
	}
}

/* Обработка логики */
static gboolean
game_logic(GtkWidget *widget)
{
	g_return_val_if_fail(widget->window, FALSE);

	/* организация движения в зависимости от нажатия клавиши */
	player.speed.x = player.speed.y = player.angular_speed = 0;
	if(keystate & KEY_W)
	{
		player.speed.y -= SPEED_STEP;
	}
	if(keystate & KEY_S)
	{
		player.speed.y += SPEED_STEP;
	}
	if(keystate & KEY_A)
	{
		player.speed.x -= SPEED_STEP;
	}
	if(keystate & KEY_D)
	{
		player.speed.x += SPEED_STEP;
	}
	if(keystate & KEY_LEFT)
	{
		player.angular_speed -= ANGULAR_STEP;
	}
	if(keystate & KEY_RIGHT)
	{
		player.angular_speed += ANGULAR_STEP;
	}

	player.angle += player.angular_speed;
	player.position.x += player.speed.x;
	player.position.y += player.speed.y;

	/* столкновения с границами */
	if(player.position.x < PLAYER_RADIUS)
	{
		player.position.x = PLAYER_RADIUS;
	}
	if(player.position.x > SCREEN_WIDTH - PLAYER_RADIUS)
	{
		player.position.x = SCREEN_WIDTH - PLAYER_RADIUS;
	}
	if(player.position.y < PLAYER_RADIUS)
	{
		player.position.y = PLAYER_RADIUS;
	}
	if(player.position.y > SCREEN_HEIGHT - PLAYER_RADIUS)
	{
		player.position.y = SCREEN_HEIGHT - PLAYER_RADIUS;
	}

	/* пуля летит */
	if(TRUE == bullet.alive)
	{
		bullet.position.x += bullet.speed.x;
		bullet.position.y += bullet.speed.y;
		/* столкновение со стеной - "смерть" пули */
		if(bullet.position.x < BULLET_RADIUS ||
		   bullet.position.x > SCREEN_WIDTH - BULLET_RADIUS ||
		   bullet.position.y < BULLET_RADIUS ||
		   bullet.position.y > SCREEN_HEIGHT - BULLET_RADIUS)
		{
			bullet.alive = FALSE;
		}
	}
	gtk_widget_queue_draw(widget);
	return TRUE;
}

/* при нажатии клавиши устанавливаем флаг кнопки... */
gboolean
on_key_press(GtkWidget *widget G_GNUC_UNUSED,
		GdkEventKey *event,
		gpointer user_data G_GNUC_UNUSED)
{
	switch(event->keyval)
	{
		case GDK_w:
			keystate |= KEY_W;
			break;
		case GDK_s:
			keystate |= KEY_S;
			break;
		case GDK_a:
			keystate |= KEY_A;
			break;
		case GDK_d:
			keystate |= KEY_D;
			break;
		case GDK_Left:
			keystate |= KEY_LEFT;
			break;
		case GDK_Right:
			keystate |= KEY_RIGHT;
			break;
		/* для пули поведение отличается - нам достаточно факта нажатия
		 * кнопки, не нужно выполнять сложные действия для вычисления
		 * состояний клавиш (но можно, если понадобится стрельба очередями:)*/
		case GDK_space:
			bullet_shot();
			break;
	}
	return FALSE;
}

/* ...при отпускании - снимаем */
gboolean
on_key_release(GtkWidget *widget G_GNUC_UNUSED,
		GdkEventKey *event,
		gpointer user_data G_GNUC_UNUSED)
{
	switch(event->keyval)
	{
		case GDK_w:
			keystate &= ~KEY_W;
			break;
		case GDK_s:
			keystate &= ~KEY_S;
			break;
		case GDK_a:
			keystate &= ~KEY_A;
			break;
		case GDK_d:
			keystate &= ~KEY_D;
			break;
		case GDK_Left:
			keystate &= ~KEY_LEFT;
			break;
		case GDK_Right:
			keystate &= ~KEY_RIGHT;
			break;
	}
	return FALSE;
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

	/* обновление экрана */
	g_signal_connect(darea, "expose-event",
			G_CALLBACK(draw_world), NULL);
	/* просто для корректного выхода */
	g_signal_connect(window, "destroy",
			G_CALLBACK(gtk_main_quit), NULL);
	/* обработка нажатий клавиш */
	g_signal_connect(G_OBJECT(window), "key_press_event",
			G_CALLBACK(on_key_press), NULL);
	g_signal_connect(G_OBJECT(window), "key_release_event",
			G_CALLBACK(on_key_release), NULL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), SCREEN_WIDTH, SCREEN_HEIGHT);

	gtk_window_set_title(GTK_WINDOW(window), "Game");
	/* 60 кадров в секунду считается приемлемым для восприятия глазом */
	g_timeout_add(1000 / FPS, (GSourceFunc)game_logic, (gpointer)window);
	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
