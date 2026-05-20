#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 골드 소비량 규칙 정의
#define COST_ADD      100  // 저항 추가 비용
#define COST_REMOVE    50  // 저항 제거 비용
#define COST_TOGGLE    30  // 직/병렬 전환 비용

typedef struct {
    int base_hp;            
    int gold;               
    int round;              
    double voltage;         
    double min_safe_current;
    double max_safe_current;
    double resistors[5];    
    int resistor_count;     
    int is_series;          
} GameState;

// 새로운 라운드 세팅 함수 (전압 및 비밀 전류 범위 리프레시)
void init_next_round(GameState *state) {
    state->round++;
    state->base_hp = 100; // 새로운 라운드가 되면 체력을 100으로 리셋
    state->gold += 200;   // 라운드 전환 보너스 골드 지급
    
    state->voltage = (rand() % 4) * 6 + 12; // 12V, 18V, 24V, 30V
    state->min_safe_current = (rand() % 6 + 3) / 10.0; // 0.3A ~ 0.8A
    state->max_safe_current = state->min_safe_current + (rand() % 5 + 3) / 10.0; 
    
    // 저항 기본값 초기화
    state->resistors[0] = 10.0;
    state->resistor_count = 1;
    state->is_series = 1;
}

// 회로 계산 및 UI 출력 함수
int update_and_render(GameState *state, int turn_passed) {
    double total_resistance = 0.0;
    double current = 0.0;
    int status_flag = 0; 

    // 1. 합성 저항 연산
    if (state->resistor_count == 0) {
        total_resistance = 9999.0; 
    } else {
        if (state->is_series) {
            for (int i = 0; i < state->resistor_count; i++) {
                total_resistance += state->resistors[i];
            }
        } else {
            double temp = 0.0;
            for (int i = 0; i < state->resistor_count; i++) {
                if (state->resistors[i] > 0) {
                    temp += (1.0 / state->resistors[i]);
                }
            }
            if (temp > 0) {
                total_resistance = 1.0 / temp;
            } else {
                total_resistance = 0.1;
            }
        }
    }

    // 2. 옴의 법칙 적용
    current = state->voltage / total_resistance;

    // 3. 화면 출력
    system("cls");
    printf("================================================================================\n");
    printf(" [ROUND %d]  |  기지 체력(HP): %d/100  |  보유 골드: %dG\n", state->round, state->base_hp, state->gold);
    printf("================================================================================\n\n");
    
    printf(" [ 실시간 전기 회로 설계도 ]\n ");
    printf("(배터리 전압: %.0fV) ---", state->voltage);
    for(int i = 0; i < state->resistor_count; i++) {
        printf("[ R%d: %.0f옴 ]---", i+1, state->resistors[i]);
    }
    printf("(GND)\n\n");

    printf("  ▶ 현재 저항 연결 방식: [%s 연결]\n", state->is_series ? "직렬" : "병렬");
    printf("  ▶ 계산된 총 합성저항: %.2f 옴\n", total_resistance);
    printf(" ------------------------------------------------------------------------------\n");
    printf(" [★ 시스템 전류 분석기 ★]\n");
    printf("  ▶ 회로에 흐르는 전류 : %.2f A\n", current);
    printf("  ※ 타워의 안전 전류 범위는 비밀입니다. 공식에 맞춰 저항을 설계하세요!\n\n");

    // 4. 4번을 눌러 턴 종료를 확정했을 때만 판정 진행
    if (turn_passed) {
        printf(" ========================= [ 라운드 결과 확인 ] =========================\n");
        printf("  ▶ 타워 요구 범위: %.2f A ~ %.2f A\n", state->min_safe_current, state->max_safe_current);
        printf("  ▶ 현재 설계 전류: %.2f A\n\n", current);

        if (current >= state->min_safe_current && current <= state->max_safe_current) {
            printf("  [성공] 안전 범위 안착! 타워가 정상 가동되어 완벽하게 방어했습니다!\n");
            status_flag = 1;
        } else if (current > state->max_safe_current) {
            printf("  [실패] 과부하(OVERLOAD) 발생! 과전류로 타워가 파괴되어 기지가 손상되었습니다!\n");
            state->base_hp -= 25;
            status_flag = 2;
        } else {
            printf("  [실패] 전류 부족! 전력이 약해 타워가 작동하지 않아 침공을 허용했습니다!\n");
            state->base_hp -= 25;
            status_flag = 2;
        }
        printf("================================================================================\n");
        system("pause");
    } else {
        printf("  [대기] 1~3번으로 설계를 변경하고, 완료되면 4번을 눌러 실행하세요.\n");
        printf("================================================================================\n");
    }

    return status_flag;
}

int main() {
    system("chcp 65001");
    srand((unsigned int)time(NULL));

    // 게임 데이터 초기화 (시작 골드 400G 지급)
    GameState game = {
        .base_hp = 100,
        .gold = 400,
        .round = 0,
        .resistors = {10.0}, 
        .resistor_count = 1,
        .is_series = 1
    };

    // 1라운드 생성 시작
    init_next_round(&game);

    int choice;
    double new_resistor;

    // 프로그램이 강제로 꺼지지 않도록 무한 루프 설정
    while (1) {
        update_and_render(&game, 0);

        printf(" [메뉴] 1.저항 추가(%dG)  2.저항 제거(%dG)  3.직/병렬 전환(%dG)  4.턴 넘기기(설계완료)\n 선택: ", 
                COST_ADD, COST_REMOVE, COST_TOGGLE);
        scanf("%d", &choice);

        switch (choice) {
            case 1: 
                if (game.resistor_count >= 5) {
                    printf("회로에 더 이상 저항을 연결할 자리가 없습니다!\n");
                    system("pause");
                } else if (game.gold < COST_ADD) {
                    printf("골드가 부족합니다!\n");
                    system("pause");
                } else {
                    printf("추가할 부품의 저항값 입력 (옴 단위): ");
                    scanf("%lf", &new_resistor);
                    if (new_resistor > 0) {
                        game.resistors[game.resistor_count++] = new_resistor;
                        game.gold -= COST_ADD;
                    }
                }
                break;

            case 2: 
                if (game.resistor_count <= 0) {
                    printf("제거할 저항 부품이 없습니다.\n");
                    system("pause");
                } else if (game.gold < COST_REMOVE) {
                    printf("골드가 부족하여 저항을 제거할 수 없습니다!\n");
                    system("pause");
                } else {
                    game.resistor_count--;
                    game.gold -= COST_REMOVE;
                    printf("마지막 저항을 제거했습니다. (-%dG)\n", COST_REMOVE);
                    system("pause");
                }
                break;

            case 3: 
                if (game.gold < COST_TOGGLE) {
                    printf("골드가 부족하여 연결 방식을 바꿀 수 없습니다!\n");
                    system("pause");
                } else {
                    game.is_series = !game.is_series;
                    game.gold -= COST_TOGGLE;
                }
                break;

            case 4: { 
                int result = update_and_render(&game, 1);
                
                // 체력이 다 깎였을 때 (게임 오버 조건 만족 시)
                if (game.base_hp <= 0) {
                    system("cls");
                    printf("====================================================\n");
                    printf(" [GAME OVER] 시스템 마비로 기지가 파괴되었습니다!\n");
                    printf(" 하지만 시스템을 재부팅하여 다음 라운드로 강제 진입합니다.\n");
                    printf("====================================================\n");
                    system("pause");
                    
                    // 꺼지지 않고 체력을 리셋하며 다음 라운드로 무조건 연장
                    init_next_round(&game);
                } 
                // 성공했거나, 실패했지만 체력이 아직 남아있을 때
                else {
                    if (result == 1) {
                        // 성공했을 때만 정상적으로 다음 라운드 진입
                        init_next_round(&game);
                    } else if (result == 2) {
                        // 실패했지만 피가 남아있으면 현재 라운드에서 저항을 재조정하도록 유도
                        printf("\n[재도전] 아직 기지 내구도가 남아있습니다! 회로를 다시 수정하세요.\n");
                        system("pause");
                    }
                }
                break;
            }
            default:
                printf("올바른 번호를 입력해주세요.\n");
                system("pause");
        }
    }

    return 0;
}

