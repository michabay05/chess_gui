#include "board.h"
#include "fen.h"
#include "gui_defs.h"

#include "raymath.h"

void draw_board(const Game* const game, Section sec)
{
    Color sq_clr, opp_clr;
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            sq_clr = SQCLR(r, f) ? LIGHT_CLR : DARK_CLR;
            opp_clr = SQCLR(r, f) ? DARK_CLR : LIGHT_CLR;
            if ((int)game->gb.selected == SQ(r, f) && pos_get_piece(game->gb.board.pos, game->gb.selected) != E)
                sq_clr = ColorAlphaBlend(sq_clr, SELECTED_COLOR, WHITE);
            if (get_bit(game->gb.preview, SQ(r, f)))
                sq_clr = ColorAlphaBlend(sq_clr, PREVIEW_COLOR, WHITE);

            DrawRectangleV(
                (Vector2) {sec.padding.x + f * sec.size.x / 8.f, sec.padding.y + r * sec.size.y / 8.f }, 
                (Vector2) {sec.size.x / 8.f, sec.size.y / 8.f},
                sq_clr
            );
            if (r == 7) {
                DrawTextEx(game->font[1], 
                           TextFormat("%c", 'a' + f), 
                           (Vector2) {
                             sec.padding.x + f * (sec.size.x / 8.f) + (sec.size.x / 8.f) - (2.5f * 0.01f * sec.size.x),
                             sec.padding.y + 0.96f * sec.size.y
                           },
                           game->font[1].baseSize * 0.55f, 0, opp_clr);
            }
        }
        DrawTextEx(game->font[1], 
                   TextFormat("%d", 8 - r), 
                   (Vector2) { sec.padding.x + 0.01 * sec.size.x, sec.padding.y + r * (sec.size.y / 8.f) + 0.01 * sec.size.y },
                   game->font[1].baseSize * 0.55f, 0, sq_clr);
    }
    DrawRectangleLines(sec.padding.x, sec.padding.y, sec.size.x, sec.size.y, RED);
}

void draw_piece(Texture2D *tex, Piece piece, int row, int col, Section sec)
{
    if (piece == E) return;

    SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
    float frame_width = tex->width / 6.0f;   // INDEX FROM ALL PIECE TYPES
    float frame_height = tex->height / 2.0f; // INDEX FOR WHITE OR BLACK
    int color = piece >= 6;
    int type = piece;

    const float SCALE = 0.9; // MAGIC NUMBER I CHOSE BECAUSE IT LOOKS GOOD
    const float SQ_SIZE = sec.size.x / 8.f;
    float texture_dim[2] = {SQ_SIZE * SCALE, SQ_SIZE * SCALE};
    float pos[2] = {
        sec.padding.x + col * SQ_SIZE + ((SQ_SIZE - texture_dim[0]) / 2.0f),
        sec.padding.y + row * SQ_SIZE + ((SQ_SIZE - texture_dim[1]) / 2.0f),
    };
    DrawTexturePro(*tex,
                   (Rectangle){type * frame_width, color * frame_height, frame_width, frame_height},
                   (Rectangle){pos[0], pos[1], texture_dim[0], texture_dim[1]}, (Vector2){0, 0}, 0, WHITE);
}

void draw_promotion_options(Texture* tex, bool is_white, Section sec)
{
    Vector2 rect_size = { sec.size.x * 0.6f, sec.size.y * 0.15f };
    Vector2 center = {
        (sec.padding.x + sec.size.x / 2.f) - (rect_size.x / 2.f),
        (sec.padding.y + sec.size.y / 2.f) - (rect_size.y / 2.f)
    };
    DrawRectangleV(
        center,
        rect_size,
        (Color) { 20, 20, 20, 150 }
    );

    SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
    for (int i = 0; i < 4; i++) {
        float frame_width = tex->width / 6.0f;   // INDEX FROM ALL PIECE TYPES
        float frame_height = tex->height / 2.0f; // INDEX FOR WHITE OR BLACK
        int color = is_white;
        int type = i + 1;

        const float SCALE = 0.85; // MAGIC NUMBER I CHOSE BECAUSE IT LOOKS GOOD
        const float SQ_SIZE = rect_size.x * 0.25f;
        float texture_dim[2] = {SQ_SIZE * SCALE, SQ_SIZE * SCALE};
        float pos[2] = { center.x + i * SQ_SIZE + ((SQ_SIZE - texture_dim[0]) / 2.0f), center.y };
        /* DrawRectangleV(
            (Vector2) { center.x + i * SQ_SIZE, center.y },
            (Vector2) { SQ_SIZE, rect_size.y },
            (Color) { 200, 0, 0, 50 }
        ); */
        DrawTexturePro(*tex,
                    (Rectangle){type * frame_width, color * frame_height, frame_width, frame_height},
                    (Rectangle){pos[0], pos[1], texture_dim[0], texture_dim[1]}, (Vector2){0, 0}, 0, WHITE);
    }
}

void choose_promotion_piece(GUI_Board* gb, Section sec)
{
    Vector2 rect_size = { sec.size.x * 0.6f, sec.size.y * 0.15f };
    Vector2 center = {
        (sec.padding.x + sec.size.x / 2.f) - (rect_size.x / 2.f),
        (sec.padding.y + sec.size.y / 2.f) - (rect_size.y / 2.f)
    };
    Vector2 mouse_pos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mouse_pos, (Rectangle){center.x, center.y, rect_size.x, rect_size.y})) {
        mouse_pos = Vector2Subtract(mouse_pos, center); 
        gb->promoted_choice = (int)(mouse_pos.x / (rect_size.x / 4.f)) + 1;
        if (gb->board.state.side == DARK) gb->promoted_choice += 6;
        gb->is_promotion = false;
   }
}

void draw_eval_bar(Font font, float eval, Section sec)
{
    const float MAGIC_FACTOR = 0.1443f;
    float scale = sqrt(fabsf(eval)) * MAGIC_FACTOR;
    scale *= eval < 0 ? 1 : -1;
    scale += 0.5;

    Color black = (Color) { 15, 15, 15, 255 };
    Color white = RAYWHITE;
    DrawRectangleV(
        sec.padding,
        (Vector2) { sec.size.x, sec.size.y * scale },
        black
    );

    DrawRectangleV(
        (Vector2) { sec.padding.x, sec.padding.y + sec.size.y * scale },
        (Vector2) { sec.size.x, sec.size.y * (1 - scale) },
        white
    );

    DrawRectangleLines(
        sec.padding.x,
        sec.padding.y,
        sec.size.x,
        sec.size.y,
        BLUE
    );

    const char* text = TextFormat("%.1f", fabsf(eval));
    float font_size = font.baseSize * 0.5f;
    Vector2 text_dim = MeasureTextEx(font, text, font_size, 0);
    Vector2 text_pos = { sec.padding.x + sec.size.x / 2.f - text_dim.x / 2.f, 0 };
    Color text_color = { 0 };
    if (eval > 0) {
        // White is winning or has an advantage
        text_pos.y = sec.padding.y + sec.size.y - (0.01 * sec.size.y + text_dim.y);
        text_color = black;
    } else {
        // Black is winning or has an advantage
        text_pos.y = sec.padding.y + 0.01f * sec.size.y;
        text_color = white;
    }

    DrawTextEx(font, text, text_pos, font_size, 0, text_color);
}

void draw_moves_list(Section sec)
{
    DrawRectangleLines(
        sec.padding.x,
        sec.padding.y,
        sec.size.x,
        sec.size.y,
        GREEN
    );
}

void draw_info(Section sec)
{
    DrawRectangleLines(
        sec.padding.x,
        sec.padding.y,
        sec.size.x,
        sec.size.y,
        WHITE
    );
}

int gui_main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess GUI");
    SetExitKey(KEY_Q);
    SetTargetFPS(30);

    Texture2D tex = LoadTexture("assets/neo-pieces-spritesheet.png");
    
    // STARTING_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    FENInfo fen = parse_fen("8/8/1k6/8/p3K3/8/8/8 w - - 0 1");
    Game game = { 0 };
    gui_board_init(&game.gb);
    board_set_from_fen(&game.gb.board, fen);

    game.font[0] = LoadFontEx("assets/font/Rubik-Regular.ttf", 30, NULL, 0);
    game.font[1] = LoadFontEx("assets/font/Rubik-Bold.ttf", 30, NULL, 0);
    for (int i = 0; i < 2; i++) SetTextureFilter(game.font[i].texture, TEXTURE_FILTER_BILINEAR);
    game.fptr = fopen("moves.txt", "w");

    Vector2 current_pos = { 0 };
    float eval = 0;
    float time = 0;
    int multiplier = 1;

    while (!WindowShouldClose()) {
        float horz_padding = GetRenderWidth() * 0.01f;
        float vert_padding = GetRenderHeight() * 0.03f;

        float eval_vert_size = fmin(0.6 * GetRenderWidth(), 0.95 * GetRenderHeight());
        game.sections[SECTION_EVAL_BAR] = (Section) {
            .size = (Vector2) { GetRenderWidth() * 0.03f, eval_vert_size},
            .padding = (Vector2) {horz_padding, vert_padding}
        };
        current_pos.x = game.sections[SECTION_EVAL_BAR].padding.x + game.sections[SECTION_EVAL_BAR].size.x;
        current_pos.y = game.sections[SECTION_EVAL_BAR].padding.y;

        float squares_area = fmin(0.6 * GetRenderWidth(), 0.95 * GetRenderHeight());
        game.sections[SECTION_BOARD] = (Section) {
            .size = (Vector2) { squares_area, squares_area },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y,
            }
        };
        current_pos.x = game.sections[SECTION_BOARD].padding.x + game.sections[SECTION_BOARD].size.x;
        current_pos.y = game.sections[SECTION_BOARD].padding.y;

        game.sections[SECTION_MOVES_LIST] = (Section) {
            .size = (Vector2) { GetRenderWidth() - (current_pos.x + 2 * horz_padding), GetRenderHeight() * 0.6f },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y
            },
        };
        current_pos.y = game.sections[SECTION_MOVES_LIST].padding.y + game.sections[SECTION_MOVES_LIST].size.y;

        game.sections[SECTION_INFO] = (Section) {
            .size = (Vector2) {
                GetRenderWidth() - (current_pos.x + 2 * horz_padding),
                GetRenderHeight() - (current_pos.y + 2 * vert_padding)
            },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y + vert_padding
            },
        };

        if (game.gb.is_promotion)
            choose_promotion_piece(&game.gb, game.sections[SECTION_BOARD]);

        gui_board_update(&game.gb, game.fptr, game.sections[SECTION_BOARD]);

        time += GetFrameTime();
        if (time > 0.01f) {
            eval += 0.05f * multiplier;
            time = 0;
        }
        if (fabsf(eval) > 11) multiplier *= -1;

        BeginDrawing();
        {
            ClearBackground(BACKGROUND);

            draw_eval_bar(game.font[1], eval, game.sections[SECTION_EVAL_BAR]);
            draw_board(&game, game.sections[SECTION_BOARD]);
            for (int i = 0; i < 64; i++) {
                draw_piece(&tex, pos_get_piece(game.gb.board.pos, i), ROW(i), COL(i), game.sections[SECTION_BOARD]);
            }
            if (game.gb.is_promotion)
                draw_promotion_options(&tex, game.gb.board.state.side, game.sections[SECTION_BOARD]);
            draw_moves_list(game.sections[SECTION_MOVES_LIST]);
            draw_info(game.sections[SECTION_INFO]);
        }
        EndDrawing();
    }

    // De-initialization
    fclose(game.fptr);
    for (int i = 0; i < 2; i++) UnloadFont(game.font[i]);
    UnloadTexture(tex);
    CloseWindow();

    return 0;
}
