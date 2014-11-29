#include <alsa/asoundlib.h>
#include <wiringPi.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
static snd_seq_t *seq_handle;
static int in_port;

// Pre-declare all functions
void allOff();
void allOn();
int choosePinIdx(int note, int channel);
void midi_open();
void midi_process(snd_seq_event_t *ev);
snd_seq_event_t *midi_read();
void myDigitalWrite(int pinIdx, int val);
void pinOff(int id, char verbose);
void pinOn(int id, char verbose);
void setPinModes(int mode);
void signalHandler(int signum);

// Constants

// The total number of pins available.
const int pinMapping[] = {
    0,
    1,
    2,
    3,
};
const int TOTAL_PINS = sizeof(pinMapping) / sizeof(int);
const int THRUPORTCLIENT = 14;
const int THRUPORTPORT = 0;

// The currently active pin.
int pinActive;

// Channel to monitor for events
int channelActive = 1;

int main()
{

    // Setup wiringPi
    if (wiringPiSetup() == -1) {
        exit(1);
    }

    // Register signal handler
    signal(SIGINT, signalHandler);

    // Setup all the pins to use OUTPUT mode
    setPinModes(OUTPUT);

    allOff();

    // Open a midi port, connect to through port also
    midi_open();

    // Process events forever
    while (true) {
        midi_process(midi_read());
    }
    
    return -1;
}

void setPinModes(int mode)
{
    int i;
    for (i = 0; i < TOTAL_PINS; i++) {
        pinMode(pinMapping[i], mode);
    }
}

void signalHandler(int signum)
{
    printf("Interrupt signal (%d) received", signum);

    // Switch the pins back to input mode
    setPinModes(INPUT);

    // Now we can exit
    exit(signum);
}

void midi_open(void)
{
    snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);

    snd_seq_set_client_name(seq_handle, "LightOrgan");
    in_port = snd_seq_create_simple_port(seq_handle, "listen:in",
                                         SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                                         SND_SEQ_PORT_TYPE_APPLICATION);

    if( snd_seq_connect_from(seq_handle, in_port, THRUPORTCLIENT, THRUPORTPORT) == -1) {
        perror("Can't connect to thru port");
        exit(-1);
    }

}

snd_seq_event_t *midi_read(void)
{
    snd_seq_event_t *ev = NULL;
    snd_seq_event_input(seq_handle, &ev);
    return ev;
}

void myDigitalWrite(int pinIdx, int val)
{
    val ? printf("%i (%i) ON\n", pinIdx, pinMapping[pinIdx]) : printf("%i (%i) OFF\n", pinIdx, pinMapping[pinIdx]);
    digitalWrite(pinMapping[pinIdx], val);
}

void pinOn(int id, char verbose='n')
{
    switch (verbose) {
        case 'y':
            myDigitalWrite(id, 0);
            break;

        case 'n':
        default:
            digitalWrite(id, 0);
            break;
    }
}

void pinOff(int id, char verbose='n')
{
    switch (verbose) {
        case 'y':
            myDigitalWrite(id, 1);
            break;

        case 'n':
        default:
            digitalWrite(id, 1);
            break;
    }
}

void allOn()
{
    int i;
    for(i=0; i< TOTAL_PINS; i++) {
        pinOn(i);
    }
}

void allOff()
{
    int i;
    for(i = 0; i< TOTAL_PINS; i++) {
        pinOff(i);
    }
}

int choosePinIdx(int note, int channel)
{
    // Return the note modulated by the number of melody pins
    int val = note  % (TOTAL_PINS * 2);
    return val / 2;
}

void midi_process(snd_seq_event_t *ev)
{

    // If this event is a PGMCHANGE type, it's a request to map a channel to an instrument
    if (ev->type == SND_SEQ_EVENT_PGMCHANGE) {
        //printf("PGMCHANGE: channel %2d, %5d, %5d\n", ev->data.control.channel, ev->data.control.param,  ev->data.control.value);

        // Clear pins state, this is probably the beginning of a new song
        allOff();
    }

    // If the event is SND_SEQ_EVENT_BOUNCE, then maybe this is an interrupt?
    if (ev->type == SND_SEQ_EVENT_BOUNCE) {
        allOff();
    }

    if (channelActive == ev->data.control.channel) {
        // Note on/off event
        if (ev->type == SND_SEQ_EVENT_NOTEON) {

            printf("Turning off pin %d\n", pinActive);

            // First turn off the current pin
            pinOff(pinActive);

            // Reset to zero if we're above the number of pins
            if (pinActive >= TOTAL_PINS) {
                printf("Resetting pinActive to zero\n");
                pinActive = 0;
            } else {
                printf("Incrementing pinActive\n");
                pinActive++;
            }

            // Turn on the next pin
            pinOn(pinActive);
        }
    }

    snd_seq_free_event(ev);
}
