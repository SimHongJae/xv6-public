#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "\n=== Setgenus Simple Test ===\n\n");
  
  // Test 1: 초기 상태
  printf(1, "1. Initial state check:\n");
  printf(1, "   getgenus() = %d (expected: -1)\n", getgenus());
  printf(1, "   getcapacity() = %d (expected: -1)\n\n", getcapacity());
  
  // Test 2: Setgenus 호출
  printf(1, "2. calling Setgenus(40):\n");
  int gid = setgenus(40);
  printf(1, "   Returned genus ID = %d\n", gid);
  printf(1, "   getgenus() = %d\n", getgenus());
  printf(1, "   getcapacity() = %d\n\n", getcapacity());
  
  // Test 3: 중복 Setgenus 시도
  printf(1, "3. Try Setgenus again (should fail):\n");
  int result = setgenus(10);
  printf(1, "   Setgenus(10) = %d (expected: -1)\n\n", result);
  
  // Test 4: fork로 상속 테스트
  printf(1, "4. Testing inheritance with fork:\n");
  int pid = fork();
  
  if(pid == 0) {
    // child process
    printf(1, "   [child] getgenus() = %d\n", getgenus());
    printf(1, "   [child] getcapacity() = %d\n", getcapacity());
    exit();
  } else {
    // Parent process
    wait();
    printf(1, "   [Parent] Still has genus = %d\n\n", getgenus());
  }
  
  // Test 5: capacity 제한 테스트
  printf(1, "5. Testing capacity limit:\n");
  pid = fork();
  
  if(pid == 0) {
    // child: 새 genus는 불가능 (이미 상속받음)
    printf(1, "   [child] Try setgenus(20) = %d (expected: -1, already has genus)\n", 
           setgenus(20));
    exit();
  } else {
    wait();
    printf(1, "   [Parent] Total capacity remains = %d\n\n", getcapacity());
  }
  
  printf(1, "\n=== Test complete ===\n\n");
  exit();
}