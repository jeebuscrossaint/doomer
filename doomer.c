#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define SPOTLIGHT_TINT_R 0
#define SPOTLIGHT_TINT_G 0
#define SPOTLIGHT_TINT_B 0
#define SPOTLIGHT_TINT_A 190

// Helper functions
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max) (MIN(MAX((x), (min)), (max)))

// Function to take a screenshot using an external program
// Returns 1 on success, 0 on failure
int take_screenshot(const char *output_path) {
  // Execute the screenshot command - here using grim as an example
  char command[256];

  // Using grim for Wayland screenshot
  snprintf(command, sizeof(command), "grim %s", output_path);

  return system(command) == 0;
}

// Vector length squared
float vector2_length_sqr(Vector2 v) { return v.x * v.x + v.y * v.y; }

int main(void) {
  // Path for saving/loading the screenshot
  const char *screenshot_path = "/tmp/spotlight_screenshot.png";

  // Take a screenshot using external tool
  printf("Taking screenshot...\n");
  if (!take_screenshot(screenshot_path)) {
    fprintf(stderr, "Failed to take screenshot\n");
    return 1;
  }

  // Initialize raylib window
  InitWindow(0, 0, "Spotlight");

  // Set window flags - using the correct constants for your raylib version
  SetWindowState(FLAG_FULLSCREEN_MODE | FLAG_WINDOW_TRANSPARENT |
                 FLAG_VSYNC_HINT);

  // Load screenshot image and texture
  Image screenshot_image = LoadImage(screenshot_path);
  Texture2D screenshot_texture = LoadTextureFromImage(screenshot_image);
  UnloadImage(screenshot_image); // Image data is now in GPU memory

  // Load spotlight shader
  Shader spotlight_shader = LoadShaderFromMemory(
      NULL, "#version 330\n"
            "in vec2 fragTexCoord;\n"
            "in vec4 fragColor;\n"
            "uniform sampler2D texture0;\n"
            "uniform vec4 colDiffuse;\n"
            "uniform vec4 spotlightTint;\n"
            "uniform vec2 cursorPosition;\n"
            "uniform float spotlightRadiusMultiplier;\n"
            "const int UNIT_RADIUS = 60;\n"
            "out vec4 finalColor;\n"
            "void main() {\n"
            "    vec4 texelColor = texture(texture0, fragTexCoord);\n"
            "    float distanceToCursor = distance(gl_FragCoord.xy, "
            "vec2(cursorPosition.x, cursorPosition.y));\n"
            "    float spotlightRadius = float(UNIT_RADIUS) * "
            "spotlightRadiusMultiplier;\n"
            "    if (distanceToCursor > spotlightRadius) {\n"
            "        finalColor = (mix(texelColor, vec4(spotlightTint.rgb, "
            "1.0), spotlightTint.a) * colDiffuse);\n"
            "    } else {\n"
            "        finalColor = (texelColor * colDiffuse);\n"
            "    }\n"
            "}");

  // Initialize camera for 2D mode
  Camera2D camera = {0};
  camera.zoom = 1.0f;

  // Initialize variables
  double delta_scale = 0.0;
  Vector2 scale_pivot = GetMousePosition();
  Vector2 velocity = {0, 0};
  float spotlight_radius_multiplier = 1.0f;
  double spotlight_radius_multiplier_delta = 0.0;

  // Get shader uniform locations
  int spotlight_tint_loc = GetShaderLocation(spotlight_shader, "spotlightTint");
  int cursor_position_loc =
      GetShaderLocation(spotlight_shader, "cursorPosition");
  int spotlight_radius_multiplier_loc =
      GetShaderLocation(spotlight_shader, "spotlightRadiusMultiplier");

  // Spotlight tint color normalized for shader
  Vector4 spotlight_tint_norm = {
      SPOTLIGHT_TINT_R / 255.0f, SPOTLIGHT_TINT_G / 255.0f,
      SPOTLIGHT_TINT_B / 255.0f, SPOTLIGHT_TINT_A / 255.0f};

  // Main game loop
  while (!WindowShouldClose() && !IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    // Toggle spotlight with left control key
    bool enable_spotlight = IsKeyDown(KEY_LEFT_CONTROL);

    // Handle mouse wheel for zooming
    float scrolled_amount = GetMouseWheelMove();

    // Initialize spotlight on first press of Ctrl
    if (IsKeyPressed(KEY_LEFT_CONTROL)) {
      spotlight_radius_multiplier = 5.0f;
      spotlight_radius_multiplier_delta = -15.0;
    }

    // Handle scroll wheel input
    if (scrolled_amount != 0.0f) {
      if (!IsKeyDown(KEY_LEFT_SHIFT)) {
        // Regular scroll adjusts zoom
        delta_scale += scrolled_amount;
      } else if (enable_spotlight) {
        // Ctrl+Shift+scroll adjusts spotlight radius
        spotlight_radius_multiplier_delta -= scrolled_amount;
      }
      scale_pivot = GetMousePosition();
    }

    // Handle zoom with smooth transitions
    if (fabs(delta_scale) > 0.5) {
      // Calculate positions before and after zoom for smooth transition
      Vector2 p0 = {scale_pivot.x / camera.zoom, scale_pivot.y / camera.zoom};

      // Update zoom with smoothing
      camera.zoom =
          (float)CLAMP(camera.zoom + delta_scale * GetFrameTime(), 1.0, 10.0);

      Vector2 p1 = {scale_pivot.x / camera.zoom, scale_pivot.y / camera.zoom};

      // Adjust camera target to keep the zoom pivot point steady
      camera.target.x += p0.x - p1.x;
      camera.target.y += p0.y - p1.y;

      // Apply damping to zoom delta
      delta_scale -= delta_scale * GetFrameTime() * 4.0;
    }

    // Update spotlight radius with smooth transitions
    spotlight_radius_multiplier =
        (float)CLAMP(spotlight_radius_multiplier +
                         spotlight_radius_multiplier_delta * GetFrameTime(),
                     0.3, 10.0);
    spotlight_radius_multiplier_delta -=
        spotlight_radius_multiplier_delta * GetFrameTime() * 4.0;

    // Handle panning with momentum
    const float VELOCITY_THRESHOLD = 15.0f;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      // Get mouse position and delta
      Vector2 mouse_pos = GetMousePosition();
      Vector2 mouse_delta = GetMouseDelta();

      // Convert screen coordinates to world coordinates
      Vector2 world_pos1 = GetScreenToWorld2D(
          (Vector2){mouse_pos.x - mouse_delta.x, mouse_pos.y - mouse_delta.y},
          camera);
      Vector2 world_pos2 = GetScreenToWorld2D(mouse_pos, camera);

      // Calculate delta in world coordinates
      Vector2 delta = {world_pos1.x - world_pos2.x,
                       world_pos1.y - world_pos2.y};

      // Update camera position
      camera.target.x += delta.x;
      camera.target.y += delta.y;

      // Set velocity for momentum effect
      velocity.x = delta.x * GetFPS();
      velocity.y = delta.y * GetFPS();
    } else if (vector2_length_sqr(velocity) >
               VELOCITY_THRESHOLD * VELOCITY_THRESHOLD) {
      // Apply momentum with deceleration
      camera.target.x += velocity.x * GetFrameTime();
      camera.target.y += velocity.y * GetFrameTime();

      // Apply friction
      velocity.x -= velocity.x * GetFrameTime() * 6.0f;
      velocity.y -= velocity.y * GetFrameTime() * 6.0f;
    }

    // Drawing
    BeginDrawing();
    BeginMode2D(camera);

    if (enable_spotlight) {
      // Apply spotlight effect

      // Set background color for spotlight mode
      Color spotlight_tint = {SPOTLIGHT_TINT_R, SPOTLIGHT_TINT_G,
                              SPOTLIGHT_TINT_B, SPOTLIGHT_TINT_A};
      ClearBackground(spotlight_tint);

      // Set shader uniform values
      SetShaderValue(spotlight_shader, spotlight_tint_loc, &spotlight_tint_norm,
                     SHADER_UNIFORM_VEC4);

      // Get mouse position
      Vector2 mouse_position = GetMousePosition();

      // Adjust cursor position for shader (shader expects Y to be from bottom,
      // raylib uses top-down coordinates)
      Vector2 cursor_position = {mouse_position.x,
                                 GetScreenHeight() - mouse_position.y};
      SetShaderValue(spotlight_shader, cursor_position_loc, &cursor_position,
                     SHADER_UNIFORM_VEC2);

      // Set spotlight radius
      SetShaderValue(spotlight_shader, spotlight_radius_multiplier_loc,
                     &spotlight_radius_multiplier, SHADER_UNIFORM_FLOAT);

      // Draw texture with shader
      BeginShaderMode(spotlight_shader);
      DrawTexture(screenshot_texture, 0, 0, WHITE);
      EndShaderMode();
    } else {
      // Normal view (no spotlight)
      ClearBackground(BLANK);
      DrawTexture(screenshot_texture, 0, 0, WHITE);
    }

    EndMode2D();
    EndDrawing();
  }

  // Clean up
  UnloadShader(spotlight_shader);
  UnloadTexture(screenshot_texture);
  CloseWindow();

  // Clean up screenshot file
  remove(screenshot_path);

  return 0;
}
