#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <SFML/Graphics.hpp>

#include "SI.hpp"
#include "CPU/i8080.hpp"


#define PREF_ASPECT_RATIO 7/8
#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 800

size_t readFile(char *filename, uint8_t **data) {
	FILE *fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("Failed to open file (%s)\n", filename);
		exit(1);
	}
	fseek(fp, 0L, SEEK_END);
	size_t filesize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	*data = (uint8_t *)malloc(filesize);
	if(data == NULL) {
		printf("Failed to allocate memory\n");
	}
	fread(*data, filesize, 1, fp);
	return filesize;
}

void handlePress(sf::RenderWindow *window, SI *machine, sf::Event keycode) {
	switch(keycode.key.code) {
		case sf::Keyboard::C: //  coin
			machine->devs[1].in |= 1 << 0; break;
		case sf::Keyboard::Num2: // P2 start
			machine->devs[1].in |= 1 << 1; break;
		case sf::Keyboard::Num1: // P1 start 
			machine->devs[1].in |= 1 << 2; break;
		case sf::Keyboard::Up: // P1 shot
			machine->devs[1].in |= 1 << 4; break;
		case sf::Keyboard::Left: // P1 Left
			machine->devs[1].in |= 1 << 5; break;
		case sf::Keyboard::Right: // P1 Right
			machine->devs[1].in |= 1 << 6; break;

		case sf::Keyboard::W: // P2 shot
			machine->devs[2].in |= 1 << 4; break;
		case sf::Keyboard::A: // P2 Left 
			machine->devs[2].in |= 1 << 5; break;
		case sf::Keyboard::D: // P2 Right 
			machine->devs[2].in |= 1 << 6; break;
		default:
			break;
	}
}


void handleUnpress(sf::RenderWindow *window, SI *machine, sf::Event keycode) {
	switch(keycode.key.code) {
		case sf::Keyboard::C:
			machine->devs[1].in &= ~(1 << 0); break;
		case sf::Keyboard::Num2: // P2 start
			machine->devs[1].in &= ~(1 << 1); break;
		case sf::Keyboard::Num1: // P1 start 
			machine->devs[1].in &= ~(1 << 2); break;
		case sf::Keyboard::Up: // P1 shot
			machine->devs[1].in &= ~(1 << 4); break;
		case sf::Keyboard::Left: // P1 Left
			machine->devs[1].in &= ~(1 << 5); break;
		case sf::Keyboard::Right: // P1 Right
			machine->devs[1].in &= ~(1 << 6); break;

		case sf::Keyboard::W: // P2 shot
			machine->devs[2].in &= ~(1 << 4); break;
		case sf::Keyboard::A: // P2 Left 
			machine->devs[2].in &= ~(1 << 5); break;
		case sf::Keyboard::D: // P2 Right 
			machine->devs[2].in &= ~(1 << 6); break;
		default:
			break;
	}
}

void input(sf::RenderWindow *window, SI *machine) {
	sf::Event event;
	while(window->pollEvent(event)) {
		switch(event.type) {
			case sf::Event::Closed:
				window->close();
				break;
			case sf::Event::KeyPressed:
				handlePress(window, machine, event);
				break;
			case sf::Event::KeyReleased:
				handleUnpress(window, machine, event);
				break;
			default:
				return;
		}
	}
}

void updateScreen(sf::RenderWindow *window, SI *machine) {
	sf::Image image;
	image.create(224, 256, sf::Color::Black);
	for(uint16_t v = 0;v<224;v++) {
		for(uint16_t h = 0;h<256;h++) {
			uint16_t v_index = v * 32; // every scanline has 32 bytes
			uint16_t h_index = h / 8; // since each byte is 8 bits, we divide by 8 and automatically take the floor of the result 
			uint8_t curByte = machine->memory[VRAM_START + v_index + h_index];
			uint8_t b = h%8;	
			sf::Color curPixel = (curByte & (1 << b)) ? sf::Color::White : sf::Color::Black;
			image.setPixel(v, 256 - h - 1, curPixel);
		}
	}
	window->clear();
	sf::Texture texture;
	texture.loadFromImage(image);
	sf::Sprite sprite;
	sprite.setPosition(0, 0);
	sprite.setTexture(texture, false);
	sf::Vector2f targetSize = window->getView().getSize();
	sprite.setScale(
			targetSize.x / sprite.getLocalBounds().width,
			targetSize.y / sprite.getLocalBounds().height
			); // making the sprite scale with any window size, its preffered to keep an aspect ratio of (224 (w)/ 256 (h)) = 7/8
	window->draw(sprite);
}

void handleIO(SI *machine) {
	if(machine->devs[4].outRead) {
		machine->devs[4].outRead = 0;
		machine->shift_reg = machine->shift_reg >> 8;
		machine->shift_reg |= ((uint16_t)(machine->devs[4].out) << 8);
	}
	if(machine->devs[2].outRead) {
		machine->devs[2].outRead = 0;
		machine->shift_offset = machine->devs[2].out & 7;
	}
	machine->devs[3].in = (machine->shift_reg >> (8 - machine->shift_offset)) & 0xFF;
}

#define INS_COUNT 1000

void mainloop(sf::RenderWindow *window, SI *machine) {
	sf::Clock clock;
	sf::Time time = sf::Time::Zero;
	long long int insCount = 0;
	while(window->isOpen() && insCount <= INS_COUNT) {
		time += clock.restart();
		size_t count = 0;
		input(window, machine);
		if(time.asMilliseconds() > 16.6667f) { // almost 60 FPS
			while(1 && insCount <= INS_COUNT) {
				size_t temp = getCycles();
				cycle();
				handleIO(machine);
				count += getCycles() - temp;
				if(count >= 16667) { 
					count -= 16666;
					interrupt(machine->current_interrupt); 
					if(machine->current_interrupt == 0xD7) {	
						window->clear();
						updateScreen(window, machine);
						machine->current_interrupt = 0xCF;	
						window->display();
						resetCycles();
						break;
					}
					machine->current_interrupt = machine->current_interrupt == 0xCF ? 0xD7 : 0xCF;
				}
			}
			time = sf::Time::Zero;
		}
	}
	window->close();
}

int main(int argc, char **argv) {
	SI machine;
	if(argc != 2) {
		printf("Usage: %s [ROM] ", argv[0]);
		return 1;
	}
	uint8_t *data;
	size_t size = readFile(argv[1], &data);
	for(size_t i=0;i<size;i++) {
		machine.memory[i] = data[i];
	}
	machine.current_interrupt = 0xCF;
	machine.shift_reg = 0;
	machine.shift_offset = 0;
	machine.devs[0].in = 0;
	machine.devs[1].in = 0;
	machine.devs[2].in = 0;
	machine.devs[3].in = 0;
	machine.devs[4].in = 0;
	machine.devs[5].in = 0;
	machine.devs[0].in = 0;
	machine.devs[0].out = 0;
	machine.devs[1].out = 0;
	machine.devs[2].out = 0;
	machine.devs[3].out = 0;
	machine.devs[4].out = 0;
	machine.devs[5].out = 0;
	machine.devs[0].outRead = 0;
	machine.devs[1].outRead = 0;
	machine.devs[2].outRead = 0;
	machine.devs[3].outRead = 0;
	machine.devs[4].outRead = 0;
	machine.devs[5].outRead = 0;

	free(data);
	initCpu(machine.memory, machine.devs);
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Space Invaders");
	mainloop(&window, &machine);
	return 0;
}
