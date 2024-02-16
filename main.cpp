#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum Coordinate
{
    x_coordinate,
    y_coordinate
};

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 1.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char YONA_SPRITE_FILEPATH[] = "yona.png";
const char HAK_SPRITE_FILEPATH[] = "Hak.png";
const char ZENO_SPRITE_FILEPATH[] = "zeno.png";
const char JAEHA_SPRITE_FILEPATH[] = "jaeha.png";
const char KIJA_SPRITE_FILEPATH[] = "kija.png";
const char SHINAH_SPRITE_FILEPATH[] = "shinah.png";
const char AO_SPRITE_FILEPATH[] = "ao.png";

const float GROWTH_FACTOR = 1.00015f;
const float SHRINK_FACTOR = 0.99985f;
const int MAX_FRAME = 1000;
SDL_Window* g_display_window;
bool g_game_is_running = true;
bool g_is_growing = true;
int g_frame_counter = 0;

ShaderProgram g_shader_program;
glm::mat4 view_matrix, g_yona_matrix, g_projection_matrix, g_trans_matrix, g_hak_matrix, g_rot_matrix, g_trans2_matrix,
g_zeno_matrix, g_jaeha_matrix, g_kija_matrix, g_shinah_matrix, g_ao_matrix;

float g_previous_ticks = 0.0f;

GLuint g_yona_texture_id;
GLuint g_hak_texture_id;
GLuint g_zeno_texture_id;
GLuint g_jaeha_texture_id;
GLuint g_kija_texture_id;
GLuint g_shinah_texture_id;
GLuint g_ao_texture_id;

float get_screen_to_ortho(float coordinate, Coordinate axis)
{
    switch (axis) {
    case x_coordinate:
        return ((coordinate / WINDOW_WIDTH) * 10.0f) - (10.0f / 2.0f);
    case y_coordinate:
        return (((WINDOW_HEIGHT - coordinate) / WINDOW_HEIGHT) * 7.5f) - (7.5f / 2.0);
    default:
        return 0.0f;
    }
}

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO); \

        g_display_window = SDL_CreateWindow("Akatsuki no Yona needs a season 2",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_yona_matrix = glm::mat4(1.0f);
    g_hak_matrix = glm::mat4(1.0f);
    g_zeno_matrix = glm::mat4(1.0f);
    g_jaeha_matrix = glm::mat4(1.0f);
    g_kija_matrix = glm::mat4(1.0f);
    g_shinah_matrix = glm::mat4(1.0f);
    g_ao_matrix = glm::mat4(1.0f);
    g_zeno_matrix = glm::translate(g_zeno_matrix, glm::vec3(-3.0f, -3.0f, 0.0f));
    g_jaeha_matrix = glm::translate(g_jaeha_matrix, glm::vec3(3.0f, -3.0f, 0.0f));
    g_kija_matrix = glm::translate(g_kija_matrix, glm::vec3(-3.0f, 3.0f, 0.0f));
    g_shinah_matrix = glm::translate(g_shinah_matrix, glm::vec3(3.0f, 3.0f, 0.0f));
    g_ao_matrix = glm::translate(g_ao_matrix, glm::vec3(4.0f, 3.0f, 0.0f));
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);
    // Notice we haven't set our model matrix yet!

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_yona_texture_id = load_texture(YONA_SPRITE_FILEPATH);
    g_hak_texture_id = load_texture(HAK_SPRITE_FILEPATH);
    g_zeno_texture_id = load_texture(ZENO_SPRITE_FILEPATH);
    g_jaeha_texture_id = load_texture(JAEHA_SPRITE_FILEPATH);
    g_kija_texture_id = load_texture(KIJA_SPRITE_FILEPATH);
    g_shinah_texture_id = load_texture(SHINAH_SPRITE_FILEPATH);
    g_ao_texture_id = load_texture(AO_SPRITE_FILEPATH);
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_game_is_running = false;
        }
    }
}

void update()
{
    g_frame_counter++;
    glm::vec3 scale_vector;

    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // compute the difference from the last frame
    g_previous_ticks = ticks;
    float rotation_angle = delta_time * DEGREES_PER_SECOND;
    if (g_frame_counter >= MAX_FRAME)
    {
        g_is_growing = !g_is_growing;
        g_frame_counter = 0;
    }

    scale_vector = glm::vec3(g_is_growing ? GROWTH_FACTOR : SHRINK_FACTOR,
        g_is_growing ? GROWTH_FACTOR : SHRINK_FACTOR,
        1.0f);
    g_yona_matrix = glm::translate(g_yona_matrix, glm::vec3(0.000125f, 0.000125f, 0.0f));
    g_hak_matrix = glm::translate(g_yona_matrix, glm::vec3(1.0f, 0.0f, 0.0f));
    g_yona_matrix = glm::rotate(g_yona_matrix, rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    g_yona_matrix = glm::scale(g_yona_matrix, scale_vector);
}



void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_yona_matrix, g_yona_texture_id);
    draw_object(g_hak_matrix, g_hak_texture_id);
    draw_object(g_zeno_matrix, g_zeno_texture_id);
    draw_object(g_jaeha_matrix, g_jaeha_texture_id);
    draw_object(g_kija_matrix, g_kija_texture_id);
    draw_object(g_shinah_matrix, g_shinah_texture_id);
    draw_object(g_ao_matrix, g_ao_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
}

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}