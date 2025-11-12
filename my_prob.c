#include "types.h"
#include "stat.h"
#include "user.h"

void test_capacity_limit(void);

int
main(int argc, char *argv[])
{
  // argc > 1이면 capacity 테스트만 실행
  if(argc > 1) {
    test_capacity_limit();
    exit();
  }
  printf(1, "\n=== Setgenus Inheritance Test ===\n\n");

  // Test 1: 초기 상태
  printf(1, "1. Initial state check:\n");
  printf(1, "   [Parent] getgenus() = %d (expected: -1)\n", getgenus());
  printf(1, "   [Parent] getcapacity() = %d (expected: -1)\n\n", getcapacity());

  // Test 2: setgenus 하기 BEFORE - fork 해도 상속 안됨
  printf(1, "2. Fork BEFORE setgenus (should NOT inherit):\n");
  int pid = fork();

  if(pid == 0) {
    // child process - genus 없어야 함
    printf(1, "   [Child] getgenus() = %d (expected: -1, NO inheritance)\n", getgenus());
    printf(1, "   [Child] getcapacity() = %d (expected: -1, NO inheritance)\n", getcapacity());

    // 자식도 자신만의 genus 설정 가능
    printf(1, "   [Child] Can set own genus: setgenus(30) = %d\n", setgenus(30));
    printf(1, "   [Child] After setgenus: getgenus() = %d\n", getgenus());
    printf(1, "   [Child] After setgenus: getcapacity() = %d\n", getcapacity());
    exit();
  } else {
    // Parent process
    wait();
    printf(1, "   [Parent] Still has no genus: getgenus() = %d\n\n", getgenus());
  }

  // Test 3: 이제 부모가 Setgenus 호출
  printf(1, "3. Parent calling Setgenus(40):\n");
  int gid = setgenus(40);
  printf(1, "   Returned genus ID = %d\n", gid);
  printf(1, "   [Parent] getgenus() = %d\n", getgenus());
  printf(1, "   [Parent] getcapacity() = %d\n\n", getcapacity());

  // Test 4: setgenus 한 AFTER - fork 하면 상속됨
  printf(1, "4. Fork AFTER setgenus (SHOULD inherit):\n");
  pid = fork();

  if(pid == 0) {
    // child process - genus 상속받아야 함
    printf(1, "   [Child] getgenus() = %d (expected: same as parent)\n", getgenus());
    printf(1, "   [Child] getcapacity() = %d (expected: same as parent)\n", getcapacity());

    // 이미 genus 있으므로 setgenus 불가
    printf(1, "   [Child] Try setgenus(20) = %d (expected: -1, already has genus)\n",
           setgenus(20));
    exit();
  } else {
    // Parent process
    wait();
    printf(1, "   [Parent] Still has genus = %d\n", getgenus());
    printf(1, "   [Parent] capacity = %d\n\n", getcapacity());
  }

  // Test 5: 중복 Setgenus 시도
  printf(1, "5. Parent try Setgenus again (should fail):\n");
  int result = setgenus(10);
  printf(1, "   Setgenus(10) = %d (expected: -1)\n\n", result);

  // Test 6: 상속받은 자식이 또 fork하면?
  printf(1, "6. Grandchild should also inherit:\n");
  pid = fork();

  if(pid == 0) {
    // child
    printf(1, "   [Child] genus = %d, capacity = %d\n", getgenus(), getcapacity());

    // 자식이 또 fork
    int pid2 = fork();
    if(pid2 == 0) {
      // grandchild
      printf(1, "   [Grandchild] genus = %d (should inherit)\n", getgenus());
      printf(1, "   [Grandchild] capacity = %d (should inherit)\n", getcapacity());
      exit();
    } else {
      wait();
      exit();
    }
  } else {
    wait();
    printf(1, "   [Parent] genus still = %d\n\n", getgenus());
  }

  printf(1, "\n=== Inheritance tests complete ===\n\n");

  // Test 7을 genus 없는 새 프로세스에서 실행
  printf(1, "Launching capacity limit test in new process...\n\n");

  pid = fork();
  if(pid == 0) {
    // 새 프로세스로 my_prob을 "capacity" 인자와 함께 실행
    char *args[] = {"my_prob", "capacity", 0};
    exec("my_prob", args);
    printf(1, "exec failed\n");
    exit();
  } else {
    wait();
  }

  printf(1, "\n=== All tests complete ===\n\n");
  exit();
}

void
test_capacity_limit(void)
{
  printf(1, "\n========================================\n");
  printf(1, "=== Total Capacity Limit Test ===\n");
  printf(1, "========================================\n\n");

  printf(1, "Initial state: genus=%d, capacity=%d\n", getgenus(), getcapacity());
  printf(1, "NOTE: Parent process from previous test still holds capacity=40\n");
  printf(1, "Creating 5 children, each trying setgenus(25)\n");
  printf(1, "Expected: Child 0,1 succeed (40+25+25=90), Child 2,3,4 fail (>90)\n\n");

  // setgenus 전에 fork 여러번 - 각자 독립적으로 genus 설정 가능
  int i;
  int child_pids[5];
  int is_child = 0;
  int child_num = -1;

  for(i = 0; i < 5; i++) {
    int pid = fork();
    if(pid == 0) {
      // Child process
      is_child = 1;
      child_num = i;
      break;
    } else {
      child_pids[i] = pid;
    }
  }

  if(is_child) {
    // Child process
    int capa = 25; // 각 자식이 25씩 요청

    // 약간의 딜레이 (순서 보장용)
    sleep(child_num * 5);

    printf(1, "   [Child %d] Attempting setgenus(%d)...\n", child_num, capa);
    int res = setgenus(capa);

    if(res >= 0) {
      printf(1, "   [Child %d] SUCCESS: genus=%d, capacity=%d\n",
             child_num, getgenus(), getcapacity());
    } else {
      printf(1, "   [Child %d] FAILED: total would exceed 90\n", child_num);
    }

    // 부모가 kill할 때까지 대기
    printf(1, "   [Child %d] Sleeping (keeping genus allocated)...\n", child_num);
    sleep(1000);
    exit();
  } else {
    // Parent process
    printf(1, "   [Parent] Waiting for children to complete setgenus...\n\n");

    // 자식들이 모두 setgenus를 시도할 시간을 줌
    sleep(50);

    // 이제 자식들을 종료시킴
    printf(1, "\n   [Parent] Killing all children...\n");
    for(i = 0; i < 5; i++) {
      kill(child_pids[i]);
    }

    // 자식들이 종료되길 기다림
    for(i = 0; i < 5; i++) {
      wait();
    }

    printf(1, "   [Parent] All children completed.\n");
    printf(1, "   Summary: Children 0,1 should succeed (40+25+25=90)\n");
    printf(1, "            Children 2,3,4 should fail (would exceed 90)\n");
  }
}
