#include "DxLib.h"
#include <string>
#include <chrono>
#include <thread>

/* ���ϐ� */
int board[8][8];        // �I�Z���Ղ̃f�[�^ (0:�Ȃ� 1:���R�} 2:���R�})
int situation[8][8];    // �I�Z���̊e�}�X�̏󋵃f�[�^ (0:�Ȃ�or�Œ� 1:�����獕�� 2:�����甒�� 3:���j�� 4:�t���O)
int mine[8][8];         // �}�C���}�b�v�̃f�[�^ (0:�n���Ȃ� -1:�n������)
int around[8][8];       // 8�ߖT�̒n�����̃f�[�^
int pro[8][8];          // �m���}�b�v (0:�Ȃ� 1:���� 2:�\������)
int disc_count[2] = { 2, 2 }; // �e�F�̃R�}�̐�
int eval[2] = { 0, 0 };       // �]���֐��l(���Ɣ�)
int tmp_eval[2] = { 0, 0 };   // �ꎞ�I�ȕ]���֐��l
int effect_wait, msg_wait;
std::string msg;
std::string mine_count;


/* �֐��錾 */
int putPiece(int x, int y, int turn, bool put_flag); // �w�肵���ʒu�ɃR�}��u��
int Make_Pro(int x, int y);       // �m���}�b�v���쐬���� (�Q�[���̐i�s�󋵂ɉ�����)
void Update_Pro();                // �m���}�b�v���A�b�v�f�[�g
void Initialize();                // �I�Z���ՂȂǂ�����������
void Remove_Mine(int x, int y);   // �n���̔��j�ɂ��ߖT�f�[�^�̃A�b�v�f�[�g������
void Set_Msg(int turn, int type); // ���b�Z�[�W�Z�b�g
bool isPass(int turn);            // �p�X�`�F�b�N
bool Think1(int turn);            // �v���C���[�̑���
bool Think2(int turn);            // CPU�̑���1 (�n���̂Ȃ��ꏊ��, �ł���������Ƃ���ɒu��)
bool Think3(int turn);            // CPU�̑���2 (�n���̂Ȃ��ꏊ��, �D�揇�ʂ̍����Ƃ���ɒu��)
void Select_Level(int* mode, int type);               // ��Փx�̑I��
int Check_Result();               // ���s�`�F�b�N


/* main�֐� */
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
    /* ���������� */
    SetGraphMode(412, 530, 32);
    ChangeWindowMode(TRUE);
    if (DxLib_Init() == -1) {
        return -1;
    }
    SetDrawScreen(DX_SCREEN_BACK);
    SetBackgroundColor(0, 255, 255);
    Initialize();

    /* �Ǐ��ϐ� */
    int TitleHandle = CreateFontToHandle(NULL, 48, 4);   // �^�C�g���̃t�H���g�̃T�C�Y�Ƒ���
    int DefaultHandle = CreateFontToHandle(NULL, 24, 3); // ��{�̃t�H���g�̃T�C�Y�Ƒ���
    int SmallHandle = CreateFontToHandle(NULL, 12, 3);   // �������t�H���g�̃T�C�Y�Ƒ���
    int status = 3; // 1:�v���C�� 2:���j�� 3:TURN���b�Z�[�W�� 4:�p�X���b�Z�[�W�� 5:�I��
    int turn = 1;   // 1:���^�[�� 2:���^�[��
    int level = 0;  // 1:Think2���g�� 2:Think3���g��
    int mode = 0;   // ���[�h�I��
    int back;       // �w�i�̃Q�[���{�[�h�摜���
    int sign[2];    // �t���O�̉摜���
    int effect[9];  // ���j�̉摜���
    int disc[8];    // �I�Z���̐΂̉摜���
    std::string player[2];

    /* �摜���̑�� */
    back = LoadGraph("board.png");
    LoadDivGraph("flag.png", 2, 2, 1, 46, 46, sign);
    LoadDivGraph("exp.png", 9, 3, 3, 48, 48, effect);
    LoadDivGraph("disc.png", 8, 2, 4, 44, 44, disc);

    /* 1�� (�^�C�g�� & ��Փx�I��) */
    while (mode < 1) {
        /* ��Փx�̑I������ */
        Select_Level(&mode, 1);
        /* �`�揈�� */
        ClearDrawScreen();
        DrawGraph(0, 50, back, FALSE);
        DrawStringToHandle(64, 184, "�}�C���I�Z��", GetColor(255, 255, 255), TitleHandle);
        DrawBox(81, 281, 181, 331, GetColor(200, 180, 150), TRUE);
        DrawBox(231, 281, 331, 331, GetColor(200, 180, 150), TRUE);
        DrawStringToHandle(88, 294, "1�lPlay", GetColor(0, 0, 0), DefaultHandle);
        DrawStringToHandle(238, 294, "2�lPlay", GetColor(0, 0, 0), DefaultHandle);
        ScreenFlip();
    }
    PlaySoundFile("select.mp3", DX_PLAYTYPE_BACK);
    while (mode == 4) {
        Select_Level(&mode, 2);
        /* �`�揈�� */
        ClearDrawScreen();
        DrawGraph(0, 50, back, FALSE);
        DrawStringToHandle(64, 184, "�}�C���I�Z��", GetColor(255, 255, 255), TitleHandle);
        DrawBox(81, 281, 181, 331, GetColor(200, 180, 150), TRUE);
        DrawBox(231, 281, 331, 331, GetColor(200, 180, 150), TRUE);
        DrawStringToHandle(96, 294, "�Ղ���", GetColor(0, 0, 0), DefaultHandle);
        DrawStringToHandle(246, 294, "���", GetColor(0, 0, 0), DefaultHandle);
        ScreenFlip();
    }
    PlaySoundFile("select.mp3", DX_PLAYTYPE_BACK);
    if (mode == 3) {
        player[0] = "   1P      ��";
        player[1] = "   2P      ��";
    }
    else {
        player[0] = " Player    ��";
        player[1] = "  CPU      ��";
    }

    /* 2�� (�I�Z���Q�[���i�s) */
    Set_Msg(turn, 0);
    while (!ProcessMessage()) {
        ClearDrawScreen();
        // �I�Z���Q�[���̐i�s
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
        /* �`�揈�� */
        // �w�i�ƃQ�[���ՊO�̕\��
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
        DrawStringToHandle(64, 516, "���N���b�N�ŋ��u��, �E�N���b�N�Ńt���b�O�𗧂Ă�", GetColor(255, 255, 255), SmallHandle);
        // �Q�[���Ղ̕\��
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
        // ���b�Z�[�W�̕\��
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


/* �֐��̒��g */
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
    int sum = 0; // �Ђ�����Ԃ����R�}���̑��a
    int kx = 0;
    int ky = 0;
    bool mine_flag = false;
    if (board[y][x] == 1 || board[y][x] == 2) {
        return 0;
    }
    if (mine[y][x] == -1) {
        mine_flag = true; // �n���̂���ꏊ�ɃR�}��u����, �Ђ�����Ԃ��������s��Ȃ�
    }
    if (!check_flag) {
        tmp_eval[turn - 1] += weight[y][x];
        tmp_eval[turn % 2] -= weight[y][x];
    }
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int wx[8], wy[8]; // ������̂Ђ�����Ԃ��\���̂���R�}�̈ʒu��ێ�.
            for (int wn = 0; wn < 8; wn++) { // wn�͈�����̂Ђ�����Ԃ��\���̂���R�}��
                kx = x + dx * (wn + 1);
                ky = y + dy * (wn + 1);
                if (kx < 0 || kx > 7 || ky < 0 || ky > 7 || board[ky][kx] == 0) {
                    break;
                }
                if (board[ky][kx] == turn) {
                    for (int i = 0; i < wn; i++) {
                        if (!check_flag) {
                            //�Ђ�����Ԃ����ꍇ�� �]���֐��l�̍X�V
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
                tmp_eval[0] = eval[0]; // �ꎞ�I�ȕ]���֐��l��߂�
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
    //�]���֐��l�̍����傫���Ƃ���ɒu��
    int max = -9999;
    int wx, wy;
    int threshold = 0;
    int num;
    while (1) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (putPiece(x, y, turn, false, false)) {
                    num = tmp_eval[turn - 1] - tmp_eval[turn % 2]; // �]���֐��l�̍�
                    if (threshold == 1 && !putPiece(x, y, 3 - turn, false, true)) {
                        num += 9999;
                    }
                    if ((max < num || (max == num && GetRand(1) == 0)) && pro[y][x] == threshold) {
                        max = num;
                        wx = x;
                        wy = y;
                    }
                }
                tmp_eval[0] = eval[0]; // �ꎞ�I�ȕ]���֐��l��߂�
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