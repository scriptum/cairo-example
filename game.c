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
#define BULLET_SPEED 5.0

/* скорость перемещения */
#define ANGULAR_STEP 0.08
#define SPEED_STEP 3.0

/* количество очков за поверженного врага */
#define ENEMY_SCORE 50
/* стоимость пули в очках */
#define BULLET_COST 10
/* скорость уменьшения счёта во времени */
#define SCORE_SPEED 2

typedef struct vec {
	gdouble x, y;
} vec;

struct {
	vec position;
	vec speed;
	gdouble angle;
	guint health;
	gint score;
	gint topscore;
} player = {
	.position = {
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2
	}, 
	.health = 100, 
	.score = 0, 
	.topscore = 0
};

struct {
	vec position;
	gdouble radius;
	gboolean alive;
} enemy;

struct {
	vec position;
	vec speed;
	gboolean alive;
} bullet = {.alive = FALSE};

/* обработка нажатий клавиш */
guint keystate = 0;
enum {
	MOVE_FORWARD  = 1 << 0,
	MOVE_BACKWARD = 1 << 1,
	ROTATE_LEFT   = 1 << 2,
	ROTATE_RIGHT  = 1 << 3,
	SHOT          = 1 << 4
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
draw_enemy(cairo_t *cr)
{
	static GdkPixbuf *enemy_image = NULL;

	if(NULL == enemy_image)
	{
		enemy_image = gdk_pixbuf_new_from_file("hell_boy.png", NULL);
		g_assert(NULL != enemy_image);
		enemy.radius = gdk_pixbuf_get_width(enemy_image) / 2;
	}
	if(FALSE == enemy.alive)
	{
		return;
	}
	cairo_save(cr);
	gdk_cairo_set_source_pixbuf(cr, enemy_image, enemy.position.x - enemy.radius, enemy.position.y - enemy.radius);
	cairo_paint(cr); 
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

static void
draw_score(cairo_t *cr)
{
	gchar buffer[32];
	cairo_move_to(cr, 5, 20);
	cairo_set_font_size(cr, 14);
	snprintf(buffer, sizeof(buffer), "Top score: %d", player.topscore);
	cairo_show_text(cr, buffer);
	snprintf(buffer, sizeof(buffer), "Score: %d", player.score);
	cairo_move_to(cr, 5, 40);
	cairo_show_text(cr, buffer);
}

/* Рисование мира */
static gboolean
draw_world(GtkWidget *widget G_GNUC_UNUSED,
		GdkEventExpose *event G_GNUC_UNUSED,
		gpointer data G_GNUC_UNUSED)
{
	cairo_t *cr;

	if(player.score > player.topscore)
	{
		player.topscore = player.score;
	}

	cr = gdk_cairo_create(widget->window);
	draw_enemy(cr);
	draw_player(cr);
	draw_bullet(cr);
	draw_score(cr);
	cairo_destroy(cr);

	return FALSE;
}

static void add_enemy()
{
	if(FALSE == enemy.alive)
	{
		enemy.alive = TRUE;
		enemy.position.x = g_random_int_range(enemy.radius, SCREEN_WIDTH - enemy.radius);
		enemy.position.y = g_random_int_range(enemy.radius, SCREEN_HEIGHT - enemy.radius);
	}
}

/* нужно для ориентации относительно "носа" игрока */
#define DELTA (M_PI_2 + M_PI_4)
/* преобразование угловых скоростей в координатные */
static vec
speed_to_vec(gdouble speed, gdouble angle)
{
	vec result;
	result.x = speed * (cos(angle - DELTA) - sin(angle - DELTA));
	result.y = speed * (sin(angle - DELTA) + cos(angle - DELTA));
	return result;
}

static void
bullet_shot()
{
	if(FALSE == bullet.alive)
	{
		bullet.speed = speed_to_vec(BULLET_SPEED, player.angle);
		bullet.speed.x += player.speed.x;
		bullet.speed.y += player.speed.y;
		bullet.position = player.position;
		bullet.alive = TRUE;
	}
}

/* Обработка логики */
static gboolean
game_logic(GtkWidget *widget)
{
	gdouble speed = 0, angular_speed = 0;

	g_return_val_if_fail(widget->window, FALSE);

	/* организация движения в зависимости от нажатия клавиши */
	if(keystate & MOVE_FORWARD)
	{
		speed += SPEED_STEP;
	}
	if(keystate & MOVE_BACKWARD)
	{
		speed -= SPEED_STEP;
	}
	if(keystate & ROTATE_LEFT)
	{
		angular_speed -= ANGULAR_STEP;
	}
	if(keystate & ROTATE_RIGHT)
	{
		angular_speed += ANGULAR_STEP;
	}
	if(keystate & SHOT)
	{
		bullet_shot();
	}

	player.angle += angular_speed;
	player.speed = speed_to_vec(speed, player.angle);
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
		gdouble dx, dy, r;
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
		/* столкновение со врагом (пересечение окружностей) */
		dx = bullet.position.x - enemy.position.x;
		dy = bullet.position.y - enemy.position.y;
		r = BULLET_RADIUS + enemy.radius;
		if(dx * dx + dy * dy < r * r)
		{
			enemy.alive = bullet.alive = FALSE;
			player.score += ENEMY_SCORE;
		}
	}

	add_enemy();

	gtk_widget_queue_draw(widget);

	return TRUE;
}
static gboolean
reduce_score(GtkWidget *widget G_GNUC_UNUSED)
{
	player.score -= SCORE_SPEED;
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
		case GDK_Up:
		case GDK_w:
			keystate |= MOVE_FORWARD;
			break;
		case GDK_Down:
		case GDK_s:
			keystate |= MOVE_BACKWARD;
			break;
		case GDK_Left:
		case GDK_a:
			keystate |= ROTATE_LEFT;
			break;
		case GDK_Right:
		case GDK_d:
			keystate |= ROTATE_RIGHT;
			break;
		case GDK_space:
			keystate |= SHOT;
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
		case GDK_Up:
		case GDK_w:
			keystate &= ~MOVE_FORWARD;
			break;
		case GDK_Down:
		case GDK_s:
			keystate &= ~MOVE_BACKWARD;
			break;
		case GDK_Left:
		case GDK_a:
			keystate &= ~ROTATE_LEFT;
			break;
		case GDK_Right:
		case GDK_d:
			keystate &= ~ROTATE_RIGHT;
			break;
		case GDK_space:
			keystate &= ~SHOT;
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
	/* каждые 100 мс уменьшаем количество очков */
	g_timeout_add(100, (GSourceFunc)reduce_score, (gpointer)window);
	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
