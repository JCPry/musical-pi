//
//  main.cpp
//  Christmas Lights
//
//  Created by Jeremy Pry on 12/25/13.
//  Copyright (c) 2013 Jeremy Pry. All rights reserved.
//

#include <alsa/asoundlib.h>
#include <wiringPi.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>
#include <iostream>

/**
 * Setup options
 */
#define NUM_PINS 4
#define NUM_CHANNELS 16
#define THRUPORTCLIENT 14
#define THRUPORTPORT 0
static snd_seq_t *seq_handle;
static int in_port;

// Currently playing note, by pin
int pin_notes[ NUM_PINS ];

// Currently playing channel, by pin
int pin_channel[ NUM_PINS ];

// Enabled channels
int play_channels[ NUM_CHANNELS ];


void midi_open( void ) {
	snd_seq_open( &seq_handle, "default", SND_SEQ_OPEN_INPUT, 0 );
	snd_seq_set_client_name( seq_handle, "JPLightOrgan" );
	in_port = snd_seq_create_simple_port( seq_handle, "listen:in", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION );

	if ( snd_seq_connect_from( seq_handle, in_port, THRUPORTCLIENT, THRUPORTPORT ) ) {
		perror( "Can't connect to thru port" );
		exit( -1 );
	}
}

snd_seq_event_t *midi_read( void ) {
	snd_seq_event_t *ev = NULL;
	snd_seq_event_input( seq_handle, &ev );
	return ev;
}

void clear_pin_notes() {
	int i;
	for ( i = 0; i <= NUM_PINS; i++ ) {
		pin_notes[ i ] = -1;
	}
}

void clear_pin_channel() {
	int i;
	for ( i = 0; i <= NUM_PINS; i++ ) {
		pin_notes[ i ] = INT_MAX;
	}
}

void clear_pins_state() {
	clear_pin_notes();
	clear_pin_channel();
}

void pins_on() {
	int i;
	for ( i = 0; i <= NUM_PINS; i++ ) {
		digitalWrite( i, 1 );
	}
}

void pins_off() {
	int i;
	for ( i = 0; i <= NUM_PINS; i++ ) {
		digitalWrite( i, 0 );
	}
}

void midi_process( snd_seq_event_t *ev ) {
	
}

int main() {

    // Start as a daemon
	if ( daemon( 0, 0 ) != 0 ) {
		exit( 1 );
	}

	// Set up WiringPi
	if ( wiringPiSetup() == -1 ) {
		exit( 1 );
	}

	// Set up all pins to use OUTPUT mode
	int i = 0;
	for ( i = 0; i <= NUM_PINS; i++ ) {
		pinMode( i, OUTPUT );
	}

	clear_pins_state();

	// Open a midi port, connect to thru port also
	midi_open();

	// Process events forever!
	while ( 1 ) {
		midi_process( midi_read() );
	}
}

