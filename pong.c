/*
C Projects #1
-------------
Title: Pong
Date: 27/12/2024
Author: Pedro Jorge
-------------
Font used: Joystix Font by Raymond Larabie
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>


#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define NUM_DOT_LINES 29
#define BALL_SIZE 20
#define PLAYER_WIDTH BALL_SIZE
#define PLAYER_HEIGHT 6*PLAYER_WIDTH
#define PLAYER1_X 3*BALL_SIZE
#define PLAYER2_X SCREEN_WIDTH-PLAYER1_X-PLAYER_WIDTH
#define PLAYER_Y (SCREEN_HEIGHT-PLAYER_HEIGHT)/2
#define MV_SPEED 10
#define BALL_VEL_X -4
#define BALL_VEL_Y 4
#define BALL_SPEED sqrt(pow(BALL_VEL_X,2) + pow(BALL_VEL_Y,2)) 
#define SPEED_INCREMENT 0.3
#define SCORE_LIMIT 11


typedef struct Velocity
{
    double x;
    double y;
} Velocity;


typedef struct Ball
{
	SDL_FRect chr;
	Velocity vel;
} Ball;


typedef struct Score
{
	int pl1;
	int pl2;
} Score;


void start_game(SDL_FRect* player1, SDL_FRect* player2, Ball* ball, double* ball_speed, int serve)
{
    // serve (1, -1) -> (pl1, pl2)
	*player1 = (SDL_FRect){PLAYER1_X,PLAYER_Y,PLAYER_WIDTH,PLAYER_HEIGHT};
	*player2 = (SDL_FRect){PLAYER2_X,PLAYER_Y,PLAYER_WIDTH,PLAYER_HEIGHT};
	*ball = (Ball){(SDL_FRect){(SCREEN_WIDTH-BALL_SIZE)/2,SCREEN_HEIGHT/2,BALL_SIZE,BALL_SIZE},
                (Velocity){serve*BALL_VEL_X, BALL_VEL_Y}};
	*ball_speed = BALL_SPEED;

}


void normalize_to_ballspeed(Velocity* vel, double ball_speed)
{
    double vel_norm = sqrt(pow(vel->x, 2) + pow(vel->y, 2));

    vel->x = ball_speed*(vel->x / vel_norm);
    vel->y = ball_speed*(vel->y / vel_norm);
}


void move_player(SDL_Renderer* renderer, SDL_FRect* player, int direction)
{
    //direction: (1, -1) -> (up, down)
    if ((direction == 1 && player->y <= 0) || (direction == -1 && player->y >= SCREEN_HEIGHT - PLAYER_HEIGHT))
    {
        return;
    }
    player->y -= direction*MV_SPEED;
}


void player_collision(SDL_Renderer* renderer, SDL_FRect player, Ball* ball, double ball_speed)
{
    // use (v_y, v_x) = (rcos, rsin)
    int r = PLAYER_HEIGHT/2;
    int d = (player.y + r) - (ball->chr.y + BALL_SIZE/2);

    // to not make ball->vel too vertical
    if (abs(d) > r - 5)
    {
        if (d > 0)
        {
            d = r - 5;
        }
        else
        {
            d = -r + 5;
        }
    }

    ball->vel.y = -d;

    if (ball->vel.x > 0)
    {
        ball->vel.x = -sqrt(pow(r,2) - pow(d,2)); 
    }
    else
    {
        ball->vel.x = sqrt(pow(r,2) - pow(d,2)); 
    }

    normalize_to_ballspeed(&ball->vel, ball_speed);
}


void draw_objects(SDL_Renderer* renderer, SDL_FRect* player1, SDL_FRect* player2, SDL_FRect* ball)
{
	SDL_FRect dotted_line = (SDL_FRect){(SCREEN_WIDTH-3)/2,0,3,SCREEN_HEIGHT};
    SDL_FRect nothing = (SDL_FRect){dotted_line.x,0,dotted_line.w,15};

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(renderer, &dotted_line);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    for (int i = 0; i < NUM_DOT_LINES-1; i++)
    {
        // N -> nothing.h ; Y -> rendered_line_lenght
        //29Y +28N = S && (i*Y + (i-1)*N)
        nothing.y = i*(SCREEN_WIDTH-NUM_DOT_LINES*nothing.h)/29+(i-1)*nothing.h;
        SDL_RenderFillRect(renderer, &nothing);
    }

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderFillRect(renderer, player1);
	SDL_RenderFillRect(renderer, player2);
	SDL_RenderFillRect(renderer, ball);
}


void draw_score(SDL_Renderer* renderer, TTF_TextEngine* engine, TTF_Font* font, Score score)
{
    char pl1_score[3];
    char pl2_score[3];
    sprintf(pl1_score, "%d", score.pl1);
    sprintf(pl2_score, "%d", score.pl2);
    
    // for font size 100
    int digit_width = 76;
    if (score.pl1 >= 10)
    {
        digit_width *= 2;
    }
    
    TTF_Text* text_pl1_score = TTF_CreateText(engine, font, &pl1_score, sizeof(pl1_score));
    TTF_Text* text_pl2_score = TTF_CreateText(engine, font, &pl2_score, sizeof(pl2_score)); 
    TTF_SetTextColor(text_pl1_score, 0xff, 0xff, 0xff, 0xc8);
    TTF_SetTextColor(text_pl2_score, 0xff, 0xff, 0xff, 0xc8); 

    TTF_DrawRendererText(text_pl1_score, SCREEN_WIDTH/2 - 101.5 - digit_width, 50);
    TTF_DrawRendererText(text_pl2_score, SCREEN_WIDTH/2 + 101.5, 50);

    TTF_DestroyText(text_pl1_score);
    TTF_DestroyText(text_pl2_score);
}


int main()
{
	SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Pong", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);	
    TTF_TextEngine* engine = TTF_CreateRendererTextEngine(renderer);
    TTF_Font* font_joystix = TTF_OpenFont("./data/joystix.ttf", 100);
    SDL_Event event;
	const bool* state = SDL_GetKeyboardState(NULL);
	
    SDL_FRect player1;
	SDL_FRect player2;
	Ball ball;
    double ball_speed; 
    int serve = 1;
    start_game(&player1, &player2, &ball, &ball_speed, serve); 
    
    Score score = (Score){0, 0};
	bool run = true;
    bool game_got_reset = true;
    
    while (run)
    {
        if (game_got_reset)
        {
            SDL_Delay(900);
            game_got_reset = !game_got_reset;
        }

        if (score.pl1 == SCORE_LIMIT || score.pl2 == SCORE_LIMIT)
        {
            run = false;
            break;
        }

        // Get Events
		SDL_PumpEvents();
		SDL_PollEvent(&event);
		if (event.type == SDL_EVENT_QUIT)
		{
		    run = false;
			break;
		}

        // Player Movement
		if (state[SDL_SCANCODE_W])
		{
			move_player(renderer, &player1, 1);
		}
		if (state[SDL_SCANCODE_S])
		{
			move_player(renderer, &player1, -1);
		}
		if (state[SDL_SCANCODE_UP])
		{
			move_player(renderer, &player2, 1);
        }
		if (state[SDL_SCANCODE_DOWN])
		{
		    move_player(renderer, &player2, -1);
        }


        // Ball Collisions with Window
        if (ball.chr.y > SCREEN_HEIGHT - BALL_SIZE)
		{
            ball.chr.y = SCREEN_HEIGHT - BALL_SIZE;
			ball.vel.y *= -1;	
		}
        else
        if (ball.chr.y < 0)
        {
            ball.chr.y = 0;
            ball.vel.y *= -1;
        }

        // Ball Collisions with Players
        // ball.vel is the error margin (because of the fps limit)
        if (ball.vel.x < 0 && PLAYER1_X + ball.vel.x <= ball.chr.x && ball.chr.x <= PLAYER1_X + PLAYER_WIDTH)
        {
            if (player1.y - BALL_SIZE <= ball.chr.y && ball.chr.y <= player1.y + PLAYER_HEIGHT)
            {
                ball_speed += SPEED_INCREMENT;
                player_collision(renderer, player1, &ball, ball_speed);
            }
        }
        else
        if (ball.vel.x > 0 && PLAYER2_X <= ball.chr.x + BALL_SIZE && ball.chr.x + BALL_SIZE <= PLAYER2_X + PLAYER_WIDTH + ball.vel.x)
        {
            if (ball.chr.y + BALL_SIZE >= player2.y && ball.chr.y <= player2.y + PLAYER_HEIGHT)
            {
                ball_speed += SPEED_INCREMENT;
                player_collision(renderer, player2, &ball, ball_speed);
            }
		}

        // Check if point was scored (hit borders)
		if (ball.chr.x >= 0 && ball.chr.x <= SCREEN_WIDTH - BALL_SIZE)
        {
			ball.chr.y += ball.vel.y;
			ball.chr.x += ball.vel.x;
		}
        else
        {
            if (ball.chr.x < 0) 
            {
			    score.pl2 += 1;
                serve = -1;
            }
            else
            if (ball.chr.x > SCREEN_WIDTH - BALL_SIZE)
            {
                score.pl1 += 1;
                serve = 1;
            }

            start_game(&player1, &player2, &ball, &ball_speed, serve);
            SDL_Delay(1500);
            game_got_reset = true;
		}

		SDL_RenderClear(renderer);

		draw_objects(renderer, &player1, &player2, &ball.chr);
        draw_score(renderer, engine, font_joystix, score);
		
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderPresent(renderer);

        SDL_Delay(10);

    }

	SDL_Quit();
    TTF_Quit();
	return 0;
}
