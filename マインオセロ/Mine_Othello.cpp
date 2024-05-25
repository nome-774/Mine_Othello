#include "DxLib.h"
#include <string>
#include <chrono>
#include <thread>

/* 大域変数 */
int board[8][8];        // オセロ盤のデータ (0:なし 1:黒コマ 2:白コマ)
int situation[8][8];    // オセロの各マスの状況データ (0:なしor固定 1:白から黒へ 2:黒から白へ 3:爆破中 4:フラグ)
int mine[8][8];         // マインマップのデータ (0:地雷なし -1:地雷あり)
int around[8][8];       // 8近傍の地雷数のデータ
int pro[8][8];          // 確率マップ (0:なし 1:あり 2:可能性あり)
int disc_count[2] = { 2, 2 }; // 各色のコマの数
int eval[2] = { 0, 0 };       // 評価関数値(黒と白)
int tmp_eval[2] = { 0, 0 };   // 一時的な評価関数値
int effect_wait, msg_wait;
std::string msg;
std::string mine_count;


/* 関数宣言 */
int putPiece(int x, int y, int turn, bool put_flag); // 指定した位置にコマを置く
int Make_Pro(int x, int y);       // 確率マップを作成する (ゲームの進行状況に応じて)
void Update_Pro();                // 確率マップをアップデート
void Initialize();                // オセロ盤などを初期化する
void Remove_Mine(int x, int y);   // 地雷の爆破による近傍データのアップデートをする
void Set_Msg(int turn, int type); // メッセージセット
bool isPass(int turn);            // パスチェック
bool Think1(int turn);            // プレイヤーの操作
bool Think2(int turn);            // CPUの操作1 (地雷のない場所で, 最も多く取れるところに置く)
bool Think3(int turn);            // CPUの操作2 (地雷のない場所で, 優先順位の高いところに置く)
void Select_Level(int* mode, int type);               // 難易度の選択
int Check_Result();               // 勝敗チェック


/* main関数 */
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
    /* 初期化処理 */
    SetGraphMode(412, 530, 32);
    ChangeWindowMode(TRUE);
    if (DxLib_Init() == -1) {
        return -1;
    }
    SetDrawScreen(DX_SCREEN_BACK);
    SetBackgroundColor(0, 255, 255);
    Initialize();

    /* 局所変数 */
    int TitleHandle = CreateFontToHandle(NULL, 48, 4);   // タイトルのフォントのサイズと太さ
    int DefaultHandle = CreateFontToHandle(NULL, 24, 3); // 基本のフォントのサイズと太さ
    int SmallHandle = CreateFontToHandle(NULL, 12, 3);   // 小さいフォントのサイズと太さ
    int status = 3; // 1:プレイ中 2:爆破中 3:TURNメッセージ中 4:パスメッセージ中 5:終了
    int turn = 1;   // 1:黒ターン 2:白ターン
    int level = 0;  // 1:Think2を使う 2:Think3を使う
    int mode = 0;   // モード選択
    int back;       // 背景のゲームボード画像情報
    int sign[2];    // フラグの画像情報
    int effect[9];  // 爆破の画像情報
    int disc[8];    // オセロの石の画像情報
    std::string player[2];

    /* 画像情報の代入 */
    back = LoadGraph("board.png");
    LoadDivGraph("flag.png", 2, 2, 1, 46, 46, sign);
    LoadDivGraph("exp.png", 9, 3, 3, 48, 48, effect);
    LoadDivGraph("disc.png", 8, 2, 4, 44, 44, disc);

    /* 1面 (タイトル & 難易度選択) */
    while (mode < 1) {
        /* 難易度の選択処理 */
        Select_Level(&mode, 1);
        /* 描画処理 */
        ClearDrawScreen();
        DrawGraph(0, 50, back, FALSE);
        DrawStringToHandle(64, 184, "マインオセロ", GetColor(255, 255, 255), TitleHandle);
        DrawBox(81, 281, 181, 331, GetColor(200, 180, 150), TRUE);
        DrawBox(231, 281, 331, 331, GetColor(200, 180, 150), TRUE);
        DrawStringToHandle(88, 294, "1人Play", GetColor(0, 0, 0), DefaultHandle);
        DrawStringToHandle(238, 294, "2人Play", GetColor(0, 0, 0), DefaultHandle);
        ScreenFlip();
    }
    PlaySoundFile("select.mp3", DX_PLAYTYPE_BACK);
    while (mode == 4) {
        Select_Level(&mode, 2);
        /* 描画処理 */
        ClearDrawScreen();
        DrawGraph(0, 50, back, FALSE);
        DrawStringToHandle(64, 184, "マインオセロ", GetColor(255, 255, 255), TitleHandle);
        DrawBox(81, 281, 181, 331, GetColor(200, 180, 150), TRUE);
        DrawBox(231, 281, 331, 331, GetColor(200, 180, 150), TRUE);
        DrawStringToHandle(96, 294, "易しい", GetColor(0, 0, 0), DefaultHandle);
        DrawStringToHandle(246, 294, "難しい", GetColor(0, 0, 0), DefaultHandle);
        ScreenFlip();
    }
    PlaySoundFile("select.mp3", DX_PLAYTYPE_BACK);
    if (mode == 3) {
        player[0] = "   1P      枚";
        player[1] = "   2P      枚";
    }
    else {
        player[0] = " Player    枚";
        player[1] = "  CPU      枚";
    }

    /* 2面 (オセロゲーム進行) */
    Set_Msg(turn, 0);
    while (!ProcessMessage()) {
        ClearDrawScreen();
        // オセロゲームの進行
        switch (status) {
        case 1:
            if (isPass(turn)) {
                Set_Msg(turn, 1);
                status = 4;
            }
            else if (mode == 1) {
                if (turn == 1) {
                    if (Think1(turn)) {
                        turn = 3 - turn;
                        status = 2;
                        Set_Msg(turn, 0);
                        PlaySoundFile("put.mp3", DX_PLAYTYPE_BACK);
                    }
                }
                else {
                    if (Think2(turn)) {
                        turn = 3 - turn;
                        status = 2;
                        Set_Msg(turn, 0);
                        std::this_thread::sleep_for(std::chrono::milliseconds(550));
                        PlaySoundFile("put.mp3", DX_PLAYTYPE_BACK);
                    }
                }
            }
            else if (mode == 2){
                if (turn == 1) {
                    if (Think1(turn)) {
                        turn = 3 - turn;
                        status = 2;
                        Set_Msg(turn, 0);
                        PlaySoundFile("put.mp3", DX_PLAYTYPE_BACK);
                    }
                }
                else {
                    if (Think3(turn)) {
                        turn = 3 - turn;
                        status = 2;
                        Set_Msg(turn, 0);
                        std::this_thread::sleep_for(std::chrono::milliseconds(550));
                        PlaySoundFile("put.mp3", DX_PLAYTYPE_BACK);
                    }
                }
            }
            else {
                if (turn == 1) {
                    if (Think1(turn)) {
                        turn = 3 - turn;
                        status = 2;
                        Set_Msg(turn, 0);
                        PlaySoundFile("put.mp3", DX_PLAYTYPE_BACK);
                    }
                }
                else {
                    if (Think1(turn)) {
                        turn = 3 - turn;
                        status = 2;
                        Set_Msg(turn, 0);
                        PlaySoundFile("put.mp3", DX_PLAYTYPE_BACK);
                    }
                }
            }
            break;
        case 2:
            if (effect_wait > 0) {
                effect_wait--;
            }
            else if (Check_Result() > 0) {
                status = 5;
            } 
            else {
                status = 3;
            }
            break;
        case 3:
            if (msg_wait > 0) {
                msg_wait--;
            }
            else {
                status = 1;
            }
            break;
        case 4:
            if (msg_wait > 0) {
                msg_wait--;
            }
            else {
                turn = 3 - turn;
                status = 3;
                Set_Msg(turn, 0);
            }
            break;
        }
        /* 描画処理 */
        // 背景とゲーム盤外の表示
        DrawGraph(0, 50, back, FALSE);
        DrawBox(206, 0, 412, 50, GetColor(204 / turn, 204 / turn, 204 / turn), TRUE);
        DrawStringToHandle(250, 13, player[1].c_str(), GetColor(255, 255, 255), DefaultHandle);
        DrawGraph(210, 3, disc[4], TRUE);
        DrawStringToHandle(350, 13, std::to_string(disc_count[1]).c_str(), GetColor(255, 255, 255), DefaultHandle);
        DrawBox(0, 462, 206, 512, GetColor(102 * turn, 102 * turn, 102 * turn), TRUE);
        DrawStringToHandle(44, 474, player[0].c_str(), GetColor(255, 255, 255), DefaultHandle);
        DrawGraph(2, 465, disc[0], TRUE);
        DrawStringToHandle(144, 474, std::to_string(disc_count[0]).c_str(), GetColor(255, 255, 255), DefaultHandle);
        DrawBox(0, 512, 412, 530, GetColor(0, 0, 0), TRUE);
        DrawStringToHandle(64, 516, "左クリックで駒を置く, 右クリックでフラッグを立てる", GetColor(255, 255, 255), SmallHandle);
        // ゲーム盤の表示
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (around[y][x] == 0) {
                    mine_count = ' ';
                }
                else {
                    mine_count = std::to_string(around[y][x]);
                }
                if (situation[y][x] == 1) {
                    DrawGraph(x * 50 + 9, y * 50 + 59, disc[(7 - effect_wait / 6)], TRUE);
                    if (effect_wait == 0) {
                        situation[y][x] = 0;
                    }
                }
                else if (situation[y][x] == 2) {
                    DrawGraph(x * 50 + 9, y * 50 + 59, disc[3 - effect_wait / 6], TRUE);
                    if (effect_wait == 0) {
                        situation[y][x] = 0;
                    }
                }
                else if (situation[y][x] == 3) {
                    DrawGraph(x * 50 + 7, y * 50 + 57, effect[9 - effect_wait / 6], TRUE);
                    if (effect_wait <= 0) {
                        situation[y][x] = 0;
                    }
                }
                else if (situation[y][x] == 4) {
                    DrawGraph(x * 50 + 7, y * 50 + 57, sign[0], TRUE);
                }
                else if (situation[y][x] == 5) {
                    DrawGraph(x * 50 + 7, y * 50 + 57, sign[1], TRUE);
                }
                else if (board[y][x] == 1) {
                    DrawGraph(x * 50 + 9, y * 50 + 59, disc[0], TRUE);
                    DrawString(x * 50 + 28, y * 50 + 74, mine_count.c_str(), GetColor(255, 255, 255));
                }
                else if (board[y][x] == 2) {
                    DrawGraph(x * 50 + 9, y * 50 + 59, disc[4], TRUE);
                    DrawString(x * 50 + 28, y * 50 + 74, mine_count.c_str(), GetColor(0, 0, 0));
                }
            }
        }
        // メッセージの表示
        if (status > 2) {
            int mw = GetDrawStringWidth(msg.c_str(), msg.size());
            DrawBox(206 - mw / 2 - 30, 234, 206 + mw / 2 + 30, 272, GetColor(200, 180, 150), TRUE);
            DrawString(206 - mw / 2, 246, msg.c_str(), GetColor(255, 255, 255));
        }
        ScreenFlip();
    }
    DxLib_End();
    return 0;
}


/* 関数の中身 */
int Make_Pro(int x, int y) {
    int update_flag = 0;
    int n = around[y][x];
    int m = 9;
    pro[y][x] = 0;
    if (n == 0) {
        for (int ky = y - 1; ky <= y + 1; ky++) {
            for (int kx = x - 1; kx <= x + 1; kx++) {
                if (kx >= 0 && kx <= 7 && ky >= 0 && ky <= 7) {
                    if (pro[ky][kx] == 2) {
                        update_flag = 1;
                    }
                    pro[ky][kx] = 0;
                }
            }
        }
    }
    else {
        for (int ky = y - 1; ky <= y + 1; ky++) {
            for (int kx = x - 1; kx <= x + 1; kx++) {
                if (kx < 0 || kx > 7 || ky < 0 || ky > 7) {
                    m--;
                }
                else if (pro[ky][kx] == 0 || pro[ky][kx] == 1) {
                    m--;
                    n -= pro[ky][kx];
                }
            }
        }
        for (int ky = y - 1; ky <= y + 1; ky++) {
            for (int kx = x - 1; kx <= x + 1; kx++) {
                if (kx >= 0 && kx <= 7 && ky >= 0 && ky <= 7 && (pro[ky][kx] != 0 && pro[ky][kx] != 1)) {
                    if (n == 0 || n == m) {
                        if (pro[ky][kx] == 2) {
                            update_flag = 1;
                        }
                        pro[ky][kx] = n / m;
                    }
                    else {
                        pro[ky][kx] = 2;
                    }
                }
            }
        }
    }
    return update_flag;
}

void Update_Pro() {
    int update_flag = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (pro[y][x] == 2) {
                pro[y][x] = -1;
            }
        }
    }
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[y][x] == 1 || board[y][x] == 2) {
                update_flag += Make_Pro(x, y);
            }
        }
    }
    if (update_flag > 0) {
        Update_Pro();
    }
}

void Initialize() {
    int mine_x, mine_y;
    int mine_num = 8 + GetRand(5);
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            board[y][x] = 0;
            situation[y][x] = 0;
            mine[y][x] = 0;
            around[y][x] = 0;
            pro[y][x] = -1;
        }
    }
    while (mine_num > 0) {
        mine_x = GetRand(7);
        mine_y = GetRand(7);
        if (mine_x != 3 && mine_x != 4 && mine_y != 3 && mine_y != 4 && mine[mine_y][mine_x] == 0) {
            mine[mine_y][mine_x] = -1;
            mine_num--;
            for (int ky = mine_y - 1; ky <= mine_y + 1; ky++) {
                for (int kx = mine_x - 1; kx <= mine_x + 1; kx++) {
                    if (kx >= 0 && kx <= 7 && ky >= 0 && ky <= 7) {
                        around[ky][kx]++;
                    }
                }
            }
        }
    }
    for (int y = 3; y < 5; y++) {
        for (int x = 3; x < 5; x++) {
            board[y][x] = (x + y + 1) % 2 + 1;
            Make_Pro(x, y);
        }
    }
    Update_Pro();
}

void Remove_Mine(int x, int y) {
    for (int ky = y - 1; ky <= y + 1; ky++) {
        for (int kx = x - 1; kx <= x + 1; kx++) {
            if (kx >= 0 && kx <= 7 && ky >= 0 && ky <= 7) {
                around[ky][kx]--;
            }
        }
    }
}

int putPiece(int x, int y, int turn, bool put_flag, bool check_flag) {
    int weight[8][8] = {
      { 30, -12,  0, -1, -1,  0, -12,  30},
      {-12, -15, -3, -3, -3, -3, -15, -12},
      {  0,  -3,  0, -1, -1,  0,  -3,   0},
      { -1,  -3, -1, -1, -1, -1,  -3,  -1},
      { -1,  -3, -1, -1, -1, -1,  -3,  -1},
      {  0,  -3,  0, -1, -1,  0,  -3,   0},
      {-12, -15, -3, -3, -3, -3, -15, -12},
      { 30, -12,  0, -1, -1,  0, -12,  30}
    };
    int sum = 0; // ひっくり返したコマ数の総和
    int kx = 0;
    int ky = 0;
    bool mine_flag = false;
    if (board[y][x] == 1 || board[y][x] == 2) {
        return 0;
    }
    if (mine[y][x] == -1) {
        mine_flag = true; // 地雷のある場所にコマを置くと, ひっくり返す処理を行わない
    }
    if (!check_flag) {
        tmp_eval[turn - 1] += weight[y][x];
        tmp_eval[turn % 2] -= weight[y][x];
    }
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int wx[8], wy[8]; // 一方向のひっくり返す可能性のあるコマの位置を保持.
            for (int wn = 0; wn < 8; wn++) { // wnは一方向のひっくり返す可能性のあるコマ数
                kx = x + dx * (wn + 1);
                ky = y + dy * (wn + 1);
                if (kx < 0 || kx > 7 || ky < 0 || ky > 7 || board[ky][kx] == 0) {
                    break;
                }
                if (board[ky][kx] == turn) {
                    for (int i = 0; i < wn; i++) {
                        if (!check_flag) {
                            //ひっくり返した場合の 評価関数値の更新
                            tmp_eval[turn - 1] += weight[wy[i]][wx[i]] * 2;
                            tmp_eval[turn % 2] -= weight[wy[i]][wx[i]] * 2;
                        }
                        if (put_flag && !mine_flag) {
                            board[wy[i]][wx[i]] = turn;
                            situation[wy[i]][wx[i]] = turn;
                        }
                    }
                    sum += wn;
                    break;
                }
                wx[wn] = kx; wy[wn] = ky;
            }
        }
    }
    if (sum > 0 && put_flag) {
        if (!mine_flag) {
            board[y][x] = turn;
            situation[y][x] = 0;
            effect_wait = 17;
            disc_count[turn - 1] += sum + 1;
            disc_count[turn % 2] -= sum;
            eval[0] = tmp_eval[0];
            eval[1] = tmp_eval[1];
            Make_Pro(x, y);
            Update_Pro();
        }
        else {
            situation[y][x] = 3;
            effect_wait = 53;
            mine[y][x] = 0;
            pro[y][x] = 0;
            Remove_Mine(x, y);
            Update_Pro();
        }
    }
    return sum;
}

bool isPass(int turn) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (putPiece(x, y, turn, false, true) != 0) {
                return false;
            }
        }
    }
    return true;
}

bool Think1(int turn) {
    static bool mouse_flag = false;
    int mx, my;
    if (GetMouseInput() & MOUSE_INPUT_LEFT) {
        if (!mouse_flag) {
            mouse_flag = true;
            GetMousePoint(&mx, &my);
            mx -= 6;
            my -= 56;
            if (mx > 0 && mx < 401 && my > 0 && my < 401) {
                if (putPiece(mx / 50, my / 50, turn, true, false)) {
                    return true;
                }
            }
        }
    }
    else if (GetMouseInput() & MOUSE_INPUT_RIGHT) {
        GetMousePoint(&mx, &my);
        mx -= 6;
        my -= 56;
        if (board[my / 50][mx / 50] == 0) {
            if (situation[my / 50][mx / 50] == 0) {
                situation[my / 50][mx / 50] = turn + 3;
            }
            else if (situation[my / 50][mx / 50] == 4 && turn == 1) {
                situation[my / 50][mx / 50] = 0;
            }
            else if (situation[my / 50][mx / 50] == 5 && turn == 2) {
                situation[my / 50][mx / 50] = 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    else {
        mouse_flag = false;
    }
    return false;
}

bool Think2(int turn) {
    int max = 0;
    int wx, wy;
    int threshold = 0;
    int num;
    while (1) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                num = putPiece(x, y, turn, false, false);
                if (threshold == 1 && !putPiece(x, y, 3 - turn, false, true)) {
                    num += 9999;
                }
                if (pro[y][x] == threshold && (max < num || (max == num && GetRand(1) == 0))) {
                    max = num;
                    wx = x;
                    wy = y;
                }
                tmp_eval[0] = eval[0]; // 一時的な評価関数値を戻す
                tmp_eval[1] = eval[1];
            }
        }
        if (max != 0) {
            break;
        }
        threshold = (threshold + 2) % 3;
    }
    putPiece(wx, wy, turn, true, false);
    return true;
}

bool Think3(int turn) {
    //評価関数値の差が大きいところに置く
    int max = -9999;
    int wx, wy;
    int threshold = 0;
    int num;
    while (1) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (putPiece(x, y, turn, false, false)) {
                    num = tmp_eval[turn - 1] - tmp_eval[turn % 2]; // 評価関数値の差
                    if (threshold == 1 && !putPiece(x, y, 3 - turn, false, true)) {
                        num += 9999;
                    }
                    if ((max < num || (max == num && GetRand(1) == 0)) && pro[y][x] == threshold) {
                        max = num;
                        wx = x;
                        wy = y;
                    }
                }
                tmp_eval[0] = eval[0]; // 一時的な評価関数値を戻す
                tmp_eval[1] = eval[1];
            }
        }
        if (max != -9999) {
            break;
        }
        threshold = (threshold + 2) % 3;
    }
    putPiece(wx, wy, turn, true, false);
    return true;
}

void Select_Level(int* mode, int type) {
    if (GetMouseInput() & MOUSE_INPUT_LEFT) {
        int x, y;
        GetMousePoint(&x, &y);
        if (y >= 281 && y <= 331) {
            if (type == 1 && x >= 80 && x <= 160) {
                *mode = 4;
            }
            else if (type == 1 && x >= 222 && x <= 302) {
                *mode = 3;
            }
            else if (type == 2 && x >= 80 && x <= 160) {
                *mode = 1;
            }
            else if (type == 2 && x >= 222 && x <= 302) {
                *mode = 2;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void Set_Msg(int turn, int type) {
    // turn ... 1:BLACK 2:WHITE 3:DRAW
    // type ... 0:TURN 1:PASS 2:WIN!
    std::string turn_str[] = { "BLACK", "WHITE", "DRAW" };
    std::string type_str[] = { "TURN", "PASS", "WIN!" };
    msg = turn_str[turn - 1];
    if (turn != 3) {
        msg += " " + type_str[type];
    }
    msg_wait = 50;
}

int Check_Result() {
    int result = 0;
    if (isPass(1) && isPass(2)) {
        if (disc_count[1] < disc_count[0]) {
            result = 1;
        }
        else if (disc_count[1] > disc_count[0]) {
            result = 2;
        }
        else {
            result = 3;
        }
    }
    if (result != 0) {
        Set_Msg(result, 2);
    }
    return result;
}