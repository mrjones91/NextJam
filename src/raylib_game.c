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
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for: 

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

// typedef enum {
//     C1,
//     C2,
//     C3,
//     C4,
//     C5
// } octaves;

int allNotes[5][7];
/*
allNotes[0][0] lowest
allNotes[0][6]

allNotes[5][0] highest
*/
// all notes implicit 2D array [5][7]
//octave[0] rainbow[0] = red and C
Color rainbow[7] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, VIOLET};
int octaves[5] = { 0, 1, 2, 3, 4};

int note = 0;
int octave = 2;

int change = 1; //connector


// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static RenderTexture2D target = { 0 };  // Render texture to render our game

// TODO: Define global variables here, recommended to make them static

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame

static int getNote(void);
static int nextNoteUp(int);
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

    if (IsKeyPressed(KEY_UP)) {
        nextNoteUp(change);
    }
    else if (IsKeyPressed(KEY_DOWN)) {
        if (!(getNote() == 0)) {
            nextNoteDown(change);
        }
    }

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture, 
    // it could be useful for scaling or further shader postprocessing
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        
        // TODO: Draw your game screen here
        DrawText("Welcome to raylib NEXT gamejam!", 150, 140, 30, BLACK);
        DrawRectangleLinesEx((Rectangle){ 0, 0, screenWidth, screenHeight }, 16, BLACK);
        TraceLog(1, "Top Color: %i", getNote() + 1);
        TraceLog(1, "Current Color: %i", getNote());
        TraceLog(1, "Bottom Color: %i", getNote() - 1);
        DrawRectangle(0, 0, screenWidth, screenHeight/3, rainbow[nextNoteUp(change) + 1]); //upper note
        DrawRectangle(0, (screenHeight/3), screenWidth, screenHeight/3, rainbow[getNote()]); //current note - middle
        DrawRectangle(0, 2 * (screenHeight/3), screenWidth, screenHeight/3, rainbow[nextNoteDown(change)]); //lower note
        DrawCircle(screenWidth/2, screenHeight/2, 40, LIGHTGRAY);

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

//get current note
int getNote() {
    return note % 7;
}

int nextNoteUp(int change) {
  //check octave. stop at [4][0]
  if (octave == 4) {
    note = 0;
  } 
  else if (octave <= 3) {
    note += change;
  }
  //check note. loop 0-6
  if (note > 6) {
    note = note % 7;
    if (octave == 3) {
        octave = 4;
    }
  }

    return note;
}
int nextNoteDown(int change) {
    //check bottom return bottom
    if (octave == 0) {
       note = 0;
    }
    else {
        note -= change;
    }
    if (note < 0) {
        note = note + 7;
    }
    return note;

}