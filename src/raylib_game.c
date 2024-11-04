/*******************************************************************************************
*
*   raylib gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: malloc(), free()
#include <string.h>                         // Required for: memcpy()
#include <math.h>           // Required for: sinf()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    SCREEN_LOGO = 0, 
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

float allNotes[5][7];
/*
allNotes[0][0] lowest
allNotes[0][6]

allNotes[5][0] highest
*/
// all notes implicit 2D array [5][7]
//octave[0] rainbow[0] = red and C

//scales
//a scale sets what seven notes are playable
//gonna stick to plain c scale -
//notes
//
/*
//calculate percentage based on change[currentChange]
        //2 step - 1.122462f || change = 1
        if (nextUp == 1 || nextUp == 2 || nextUp == 4 || nextUp == 5 || nextUp == 6){
            frequency = frequency * 1.122462f;
        } else {
            frequency = frequency * 1.059463f;
        }

        if (nextDown == 0 || nextDown == 1 || nextDown == 3 || nextDown == 4 || nextDown == 5){
            frequency = frequency / (change[currentChange] + nextDown * 1.059463f);
        } else {
            frequency = frequency / 1.059463f;
        }

        //change = 2 * 1.122462f
        //change = 3 * 1.122462f
        //change = 4 
        //change = 7
*/


Color rainbow[7] = {(Color){230, 41,55, 200}, (Color){255, 161, 0, 200}, (Color){253, 249, 0, 200}, (Color){0, 228, 48, 200}, (Color){0, 121,241, 200}, (Color){200, 122, 255, 200}, (Color){135, 60, 190, 200}};
Color rainbows[7] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, VIOLET};
char notes[7] = { 'C', 'D', 'E', 'F', 'G', 'A', 'B'};
int octaves[5] = { 0, 1, 2, 3, 4};
int change[5] = {1, 2, 3, 4, 7};


int note = 0;
int octave = 2;

int currentChange = 0; //connector

int tune[8]; //play should stack notes played here...

#define MAX_SAMPLES               512
#define MAX_SAMPLES_PER_UPDATE   4096

// Cycles per second (hz)
float frequency = 61.74f;

// Audio frequency, for smoothing
float audioFrequency = 262.0f;

// Previous value, used to test if sine needs to be rewritten, and to smoothly modulate frequency
float oldFrequency = 1.0f;

// Index for audio rendering
float sineIdx = 0.0f;

// Init raw audio stream (sample rate: 44100, sample size: 16bit-short, channels: 1-mono)
AudioStream stream;

// Audio input processing callback
void AudioInputCallback(void *buffer, unsigned int frames)
{
    audioFrequency = frequency + (audioFrequency - frequency)*0.95f;

    float incr = audioFrequency/44100.0f;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++)
    {
        d[i] = (short)(32000.0f*sinf(2*PI*sineIdx));
        sineIdx += incr;
        if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }
}




// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static RenderTexture2D target = { 0 };  // Render texture to render our game

Vector2 touchPosition = { 0, 0 };
Rectangle touchArea = { 0, 0, screenWidth, screenHeight};

Font font1;


// TODO: Define global variables here, recommended to make them static

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame

static void loadNotes(void);

static int getNote(int);
static int moveUp(int);
static int nextNoteUp(int);
static int moveDown(int);
static int nextNoteDown(int);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");
    
    loadNotes(); //init all note values

    frequency = allNotes[octave][note];

    InitAudioDevice();              // Initialize audio device

    SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);

    // Init raw audio stream (sample rate: 44100, sample size: 16bit-short, channels: 1-mono)
    stream = LoadAudioStream(44100, 16, 1);

    SetAudioStreamCallback(stream, AudioInputCallback);

    // Buffer for the single cycle waveform we are synthesizing
    short *data = (short *)malloc(sizeof(short)*MAX_SAMPLES);

    // Frame buffer, describing the waveform when repeated over the course of a frame
    short *writeBuf = (short *)malloc(sizeof(short)*MAX_SAMPLES_PER_UPDATE);

    PlayAudioStream(stream);        // Start processing stream buffer (no data loaded currently) 

    font1 = LoadFont("resources/alpha_beta.png");
    // TODO: Load resources / Initialize variables at this point
    
    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    free(data);                 // Unload sine wave data
    free(writeBuf);             // Unload write buffer

    UnloadAudioStream(stream);   // Close raw audio stream and delete buffers from RAM
    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)
    
    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
// Update and draw frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    //----------------------------------------------------------------------------------

    if (IsKeyPressed(KEY_SPACE) || (GetGestureDetected() == GESTURE_TAP)){
        if (IsAudioStreamPlaying(stream)) {
            PauseAudioStream(stream);
        } else {
            ResumeAudioStream(stream);
        }
    }
    if (IsKeyPressed(KEY_UP) || (GetGestureDetected() == GESTURE_SWIPE_DOWN))  {
       note = moveUp(change[currentChange]);
       //(int)(frequency * 2) ^ (change[currentChange] / 12);
    }
    else if (IsKeyPressed(KEY_DOWN) || (GetGestureDetected() == GESTURE_SWIPE_UP))  {
        note = moveDown(change[currentChange]);
        //frequency = frequency / 1.059463f;//(int)(frequency * 2) ^ (-1 * change[currentChange] / 12);
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        if (currentChange < (int) ( sizeof(change) / sizeof(change[0]) ) - 1 ) {
            currentChange += 1;
        }
    }

    if (IsKeyPressed(KEY_LEFT)) {
        if (currentChange > 0) {
            currentChange -= 1;
        }
    }

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture, 
    // it could be useful for scaling or further shader postprocessing
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        
        // TODO: Draw your game screen here
        //DrawRectangleLinesEx((Rectangle){ 0, 0, screenWidth, screenHeight }, 16, DARKGRAY);
    
        DrawRectangle(0, 0, screenWidth, screenHeight/3, rainbow[nextNoteUp(change[currentChange])]); //upper note
        DrawRectangle(0, (screenHeight/3), screenWidth, screenHeight/3, rainbow[getNote(0)]); //current note - middle
        DrawRectangle(0, 2 * (screenHeight/3), screenWidth, screenHeight/3, rainbow[nextNoteDown(change[currentChange])]); //lower note
        DrawCircle(screenWidth/2, screenHeight/2, 40, DARKGRAY);

        //DrawText("Welcome to raylib NEXT gamejam!", 150, 140, 30, BLACK);
        
        char displayText1[50];// = "Note: &s, Octave: %i" + player.position.x;
        char displayText2[30];// = "Note: &s, Octave: %i" + player.position.x;
        char displayText3[30];// = "Note: &s, Octave: %i" + player.position.x;
        char displayText4[40];// = "Note: &s, Octave: %i" + player.position.x;
        char displayText5[40];// = "Note: &s, Octave: %i" + player.position.x;
        sprintf(displayText1, "Note Name: %c, Octave: %i, Note #: %i", notes[note], octave, note);
        sprintf(displayText2, "Change: %i,", change[currentChange]);
        sprintf(displayText3, "Frequency: %f", frequency);
        sprintf(displayText4, "Scale: C Major");

        // DrawRectangle(15, 150, 290, 110, DARKGRAY);
        // DrawText(displayText1, 20, 160, 15, rainbow[nextNoteUp(change[2])]);
        // DrawText(displayText2, 20, 180, 15, rainbow[nextNoteUp(change[2])]);
        // DrawText(displayText3, 20, 200, 15, rainbow[nextNoteUp(change[2])]);
        // DrawText(displayText4, 20, 220, 15, rainbow[nextNoteUp(change[2])]);
        if (IsAudioStreamPlaying(stream)){
            sprintf(displayText5, "TAP or Press SPACE to PAUSE Audio");
        } else {
            sprintf(displayText5, "TAP or Press SPACE to Resume Audio");
        }
        //DrawText(displayText5, 20, 240, 15, rainbow[nextNoteUp(change[2])]);
        DrawTextEx(font1, displayText1, (Vector2){ 20, 160}, (float)font1.baseSize * 1.5, 2, DARKGRAY);
        DrawTextEx(font1, displayText2, (Vector2){ 20, 180}, (float)font1.baseSize * 1.5, 2, DARKGRAY);
        DrawTextEx(font1, displayText3, (Vector2){ 20, 200}, (float)font1.baseSize * 1.5, 2, DARKGRAY);
        DrawTextEx(font1, displayText4, (Vector2){ 20, 220}, (float)font1.baseSize * 1.5, 2, DARKGRAY);
        DrawTextEx(font1, displayText5, (Vector2){ 20, 240}, (float)font1.baseSize * 1.5, 2, DARKGRAY);
        
    EndTextureMode();
    
    // Render to screen (main framebuffer)
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw render texture to screen, scaled if required
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, (Rectangle){ 0, 0, (float)target.texture.width, (float)target.texture.height }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
    //----------------------------------------------------------------------------------  
}
// void loadNotes() {
//     for (int i = 0; i < allNotes[1][1].length; i++){
//         for (int j = 0; j < allNotes)
//     }
// }

void loadNotes() {
    float tempFreq = frequency;

    for (int o = 0; o < 5; o++) {
        for (int n = 0; n < 7; n++) {

            if (n == 0 || n == 3) {
                tempFreq *= 1.059463f;
            } else {
                tempFreq *= 1.122462f;
            }
            
            allNotes[o][n] = tempFreq;
        }
    }
    
}

//get current note
int getNote(int diff) {
    if (diff > 0) {
        return note + diff % 7;

    } else if (diff < 0) {
        return note + diff + 7;
    } else {
        return note;
    }
}

int moveUp(int change) {
    int nextUp = note + change;
    //int fc = 1;
    if (nextUp > 6) {
        if (octave < 4){
            octave += 1;
        } else if (octave == 4) {
            nextUp = 6;
        }
        nextUp = nextUp % 7;
    }
    if (!(note == 6 && octave == 4)) {
        // //calculate percentage based on change[currentChange]
        // //2 step - 1.122462f
        // if (nextUp == 1 || nextUp == 2 || nextUp == 4 || nextUp == 5 || nextUp == 6){
        //     fc += change - 1;//frequency = frequency * 1.122462f;
        // } else {
        //     fc += 1;
        //     //frequency = frequency * 1.059463f;
        // }
        frequency = allNotes[octave][note];
    }
    return nextUp;
}
int nextNoteUp(int change) {
    return (note + change) % 7;
}
int moveDown(int change) {
    //check bottom return bottom
    //int fc = 1;
    int nextDown = note - change;
    if (nextDown < 0) {
        if (octave > 0){
            octave -= 1;
        } else if (octave == 0) {
            nextDown = 0;
        }
        nextDown = (nextDown + 7) % 7;
    }
    //frequency * (change * .06 + 1)
    //frequency / (change * .06 + 1)
    if (!(note == 0 && octave == 0)) {
        // if (nextDown == 0 || nextDown == 1 || nextDown == 3 || nextDown == 4 || nextDown == 5){
        //     //if change > 1 , change += change - 1?
        //     fc += change - 1;
        //     //frequency = frequency / 1.122462f;
        // } else {
        //     fc += 1;
        //     //frequency = frequency / 1.059463f;
        // }
        frequency = allNotes[octave][note];
        //frequency = frequency / (.059463f * fc + 1);
    }
    return nextDown;
}
int nextNoteDown(int change) {
    return ((note - change) + 7) % 7;
}