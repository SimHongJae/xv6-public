#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "\n=== SetGenus Simple Test ===\n\n");
  
  // Test 1: 초기 상태
  printf(1, "1. Initial state check:\n");
  printf(1, "   GetGenus() = %d (expected: -1)\n", GetGenus());
  printf(1, "   GetCapacity() = %d (expected: -1)\n\n", GetCapacity());
  
  // Test 2: SetGenus 호출
  printf(1, "2. Calling SetGenus(40):\n");
  int gid = SetGenus(40);
  printf(1, "   Returned genus ID = %d\n", gid);
  printf(1, "   GetGenus() = %d\n", GetGenus());
  printf(1, "   GetCapacity() = %d\n\n", GetCapacity());
  
  // Test 3: 중복 SetGenus 시도
  printf(1, "3. Try SetGenus again (should fail):\n");
  int result = SetGenus(10);
  printf(1, "   SetGenus(10) = %d (expected: -1)\n\n", result);
  
  // Test 4: fork로 상속 테스트
  printf(1, "4. Testing inheritance with fork:\n");
  int pid = fork();
  
  if(pid == 0) {
    // Child process
    printf(1, "   [Child] GetGenus() = %d\n", GetGenus());
    printf(1, "   [Child] GetCapacity() = %d\n", GetCapacity());
    exit();
  } else {
    // Parent process
    wait();
    printf(1, "   [Parent] Still has genus = %d\n\n", GetGenus());
  }
  
  // Test 5: Capacity 제한 테스트
  printf(1, "5. Testing capacity limit:\n");
  pid = fork();
  
  if(pid == 0) {
    // Child: 새 genus는 불가능 (이미 상속받음)
    printf(1, "   [Child] Try SetGenus(20) = %d (expected: -1, already has genus)\n", 
           SetGenus(20));
    exit();
  } else {
    wait();
    printf(1, "   [Parent] Total capacity remains = %d\n\n", GetCapacity());
  }
  
  printf(1, "\n=== Test Complete ===\n\n");
  exit();
}