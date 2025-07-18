#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#define color_toparam(color) color.r, color.g, color.b, color.a

typedef struct button {
	char* text;
	TTF_Font* font;
	int font_size;
	SDL_Color text_color, color, hover_color;
	SDL_Rect text_canvas, canvas;
	bool clicked;
} button_t;

void button_getsize(button_t* button) {
	TTF_SetFontSize(button->font, button->font_size);
	TTF_SizeText(button->font, button->text, &button->text_canvas.w, &button->text_canvas.h);
	button->canvas = button->text_canvas;
}

bool button_hover(int mouse_x, int mouse_y, button_t button) {
	SDL_Rect canvas = button.canvas;
	if((mouse_x <= canvas.x + canvas.w && mouse_x >= canvas.x)
	    && (mouse_y <= canvas.y + canvas.h && mouse_y >= canvas.y)) {
		return true;
	}
	return false;
}

void button_render(SDL_Renderer* renderer, int mouse_x, int mouse_y, bool* mouse_clicked,
	button_t* button) {
	if(button_hover(mouse_x, mouse_y, *button)) {
		SDL_SetRenderDrawColor(renderer, color_toparam(button->hover_color));
		if((*mouse_clicked)) {
			button->clicked = true;
			(*mouse_clicked) = false;
		}
	} else {
		SDL_SetRenderDrawColor(renderer, color_toparam(button->color));
	}
	SDL_RenderDrawRect(renderer, &button->canvas);
	SDL_RenderFillRect(renderer, &button->canvas);
	SDL_Surface* button_text_surface = TTF_RenderText_Blended(button->font, button->text, button->text_color);
	SDL_Texture* button_text_texture = SDL_CreateTextureFromSurface(renderer, button_text_surface);
	SDL_FreeSurface(button_text_surface);
	SDL_RenderCopy(renderer, button_text_texture, NULL, &button->text_canvas);
	SDL_DestroyTexture(button_text_texture);
}

void buttons_getsizes(button_t* buttons, size_t button_count) {
	for(size_t i=0;i<button_count;i++) {
		button_getsize(&buttons[i]);
	}
}

void render_buttons(SDL_Renderer* renderer, int mouse_x, int mouse_y, bool mouse_clicked,
	button_t* buttons, size_t button_count) {
	for(size_t i=0;i<button_count;i++) {
		button_render(renderer, mouse_x, mouse_y, &mouse_clicked, &buttons[i]);
	}
}

void appendstr(char** string, size_t* string_len, const char* source) {
	size_t new_count = (*string_len) + strlen(source);
	char* new_ptr = realloc(*string, new_count + 1);
	strcpy(new_ptr + (*string_len), source);
	new_ptr[new_count] = '\0';
	*string = new_ptr;
	*string_len = new_count;
}

bool isoperator(char* string) {
	const char* operators = "+-/*";
	while(*operators) {
		if(string[0] == *operators) {
			return true;
		}
		operators++;
	}
	return false;
}

void inputbox_clear(char** input_text, size_t* input_textlen) {
	free((*input_text));
	*input_text = NULL;
	*input_textlen = 0;
}

size_t get_numlength(long num) {
	size_t num_length = 1;
	while(num >= 10) {
		num_length++;
		num /= 10;
	}
	return num_length;
}

long eval_expr(int num1, char operator, int num2) {
	long result = 0;
	if(operator == '+') {
		result = num1 + num2;
	} else if(operator == '-') {
		result = num1 - num2;
	} else if(operator == '/') {
		result = num1 / num2;
	} else if(operator == '*') {
		result = num1 * num2;
	}
	return result;
}

char* eval_exprtostr(int num1, char operator, int num2,
	size_t* result_length) {
	long result = eval_expr(num1, operator, num2);
	size_t result_len = get_numlength(result);
	char* result_str = calloc(result_len + 1, sizeof(char));
	sprintf(result_str, "%ld", result);
	*result_length = result_len;
	return result_str;
}

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	bool quit = false,
	     mouse_clicked = false;
	SDL_Event e;
	int mouse_x = 0, mouse_y = 0, win_w = 900, win_h = 600;
	SDL_Window* window = SDL_CreateWindow("Calculator",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			win_w, win_h, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Color window_color = {0x12, 0x12, 0x12, 0x12};
	int font_size = 36;
	TTF_Font* font = TTF_OpenFont("Symbola.ttf", font_size);
	char* input_text = NULL;
	size_t input_textlen = 0;
	SDL_Rect inputbox = {.y = 10, .h = 100},
		 input_textcanvas = {0};
	SDL_Color inputbox_color = {0x00, 0xff, 0x00, 0xff},
		  input_textcolor = {0xff, 0xff, 0xff, 0xff};
	button_t buttons[] = {
		{"7"}, {"8"}, {"9"}, {"+"},
		{"4"}, {"5"}, {"6"}, {"-"},
		{"1"}, {"2"}, {"3"}, {"/"},
		{"Clear"}, {"0"},  {"="}, {"*"}
	};
	int nums[2] = {0};
	size_t num_index = 0;
	bool eval = false, result_ready = false;
	char operator = '\0';
	size_t button_count = sizeof(buttons) / sizeof(button_t);
	for(size_t i=0;i<button_count;i++) {
		buttons[i].font = font;
		buttons[i].font_size = font_size;
		buttons[i].color = (SDL_Color){0};
		buttons[i].hover_color = (SDL_Color){0xff, 0x00, 0x00, 0xff};
		buttons[i].text_color = (SDL_Color){0xff, 0xff, 0xff, 0xff};
	}
	buttons_getsizes(buttons, button_count);
	while(!quit) {
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				quit = 1;
			} else if(e.type == SDL_MOUSEMOTION) {
				mouse_x = e.motion.x, mouse_y = e.motion.y;
			} else if(e.type == SDL_MOUSEBUTTONDOWN ||
				e.type == SDL_MOUSEBUTTONUP) {
				mouse_x = e.button.x, mouse_y = e.button.y;
				if(e.type == SDL_MOUSEBUTTONUP) {
					mouse_clicked = true;
				}
			}
		}
		SDL_GetWindowSize(window, &win_w, &win_h);
		SDL_SetRenderDrawColor(renderer, color_toparam(window_color));
		SDL_RenderClear(renderer);
		inputbox.w = win_w - 20;
		inputbox.x = (win_w - inputbox.w) / 2;
		SDL_SetRenderDrawColor(renderer, color_toparam(inputbox_color));
		SDL_RenderDrawRect(renderer, &inputbox);
		
		int start_x = 0, start_y = inputbox.y + inputbox.h,
			max_button_w = win_w / 4,
			max_button_h = (win_h - (inputbox.y + inputbox.h)) / 4;
		int count = 0;
		for(size_t i=0;i<button_count;i++) {
			SDL_Rect *canvas = &buttons[i].canvas,
				 *text_canvas = &buttons[i].text_canvas;
			canvas->x = start_x, canvas->y = start_y;
			canvas->w = max_button_w - 5, canvas->h = max_button_h - 5;
			text_canvas->x = canvas->x + (canvas->w - text_canvas->w) / 2;
			text_canvas->y = canvas->y + (canvas->h - text_canvas->h) / 2;
			count++;
			if(count >= 4) {
				start_x = 0;
				start_y += max_button_h;
				count = 0;
				continue;
			}
			start_x += max_button_w;
		}
		render_buttons(renderer, mouse_x, mouse_y, mouse_clicked, buttons, button_count);
		for(size_t i=0;i<button_count;i++) {
			if(isdigit(buttons[i].text[0]) && buttons[i].clicked) {
				if(!num_index && result_ready) {
					nums[0] = atoi(input_text);
					num_index = 1;
					inputbox_clear(&input_text, &input_textlen);
					result_ready = false;
				}
				appendstr(&input_text, &input_textlen, buttons[i].text);
				nums[num_index] = atoi(input_text);
			} else if(isoperator(buttons[i].text) && buttons[i].clicked) {
				num_index = (num_index + 1) % 2;
				if(!num_index) {
					inputbox_clear(&input_text, &input_textlen);
					input_text = eval_exprtostr(nums[0], operator, nums[1], &input_textlen);
					result_ready = true;
				} else if(num_index == 1) {
					inputbox_clear(&input_text, &input_textlen);
				}
				operator = buttons[i].text[0];
			} else if(strcmp(buttons[i].text, "=") == 0 && buttons[i].clicked) {
				eval = true;
			} else if(strcmp(buttons[i].text, "Clear") == 0 && buttons[i].clicked) {
				eval = false;
			}
			buttons[i].clicked = false;
		}
		if(eval && num_index == 1) {
			inputbox_clear(&input_text, &input_textlen);
			input_text = eval_exprtostr(nums[0], operator, nums[1], &input_textlen);
			nums[0] = atoi(input_text);
			num_index = 0;
		}
		if(input_text) {
			TTF_SetFontSize(font, font_size);
			TTF_SizeText(font, input_text, &input_textcanvas.w, &input_textcanvas.h);	
			input_textcanvas.x = inputbox.x + 1;
			input_textcanvas.y = inputbox.y + (inputbox.h - input_textcanvas.h) / 2;
			SDL_Surface* surface = TTF_RenderText_Blended(font, input_text, input_textcolor);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface);
			SDL_RenderCopy(renderer, texture, NULL, &input_textcanvas);
			SDL_DestroyTexture(texture);
		}
		mouse_clicked = false;
		SDL_RenderPresent(renderer);
		eval = false;
	}
	inputbox_clear(&input_text, &input_textlen);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
