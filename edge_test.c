#include "types.h"
#include "stat.h"
#include "user.h"

// Edge case tests for setgenus implementation

void test_invalid_inputs(void);
void test_boundary_values(void);
void test_capacity_accounting(void);
void test_nested_inheritance(void);
void test_exec_scenarios(void);
void test_race_conditions(void);
void test_killed_processes(void);
void test_zombie_processes(void);
void test_genus_id_wraparound(void);

// Helper function to run test in isolated process
void run_test(void (*test_func)(void))
{
  int pid = fork();
  if(pid == 0) {
    test_func();
    exit();
  }
  wait();
}

int
main(int argc, char *argv[])
{
  printf(1, "\n========================================\n");
  printf(1, "=== Setgenus Edge Case Tests ===\n");
  printf(1, "========================================\n\n");

  run_test(test_invalid_inputs);
  run_test(test_boundary_values);
  run_test(test_capacity_accounting);
  run_test(test_nested_inheritance);
  run_test(test_exec_scenarios);
  run_test(test_race_conditions);
  run_test(test_killed_processes);
  run_test(test_zombie_processes);
  run_test(test_genus_id_wraparound);

  printf(1, "\n========================================\n");
  printf(1, "=== All edge case tests complete ===\n");
  printf(1, "========================================\n\n");
  exit();
}

void
test_invalid_inputs(void)
{
  printf(1, "TEST 1: Invalid Input Tests\n");
  printf(1, "----------------------------\n");

  // Test negative capacity
  int result = setgenus(-10);
  printf(1, "   setgenus(-10) = %d %s\n", result,
         result == -1 ? "[PASS]" : "[FAIL]");

  // Test zero capacity
  result = setgenus(0);
  printf(1, "   setgenus(0) = %d %s\n", result,
         result == -1 ? "[PASS]" : "[FAIL]");

  // Test very large capacity (exceeds max)
  result = setgenus(100);
  printf(1, "   setgenus(100) = %d %s\n", result,
         result == -1 ? "[PASS]" : "[FAIL]");

  // Test setgenus(91) when total=0 (should fail)
  result = setgenus(91);
  printf(1, "   setgenus(91) = %d %s\n", result,
         result == -1 ? "[PASS]" : "[FAIL]");

  printf(1, "\n");
}

void
test_boundary_values(void)
{
  printf(1, "TEST 2: Boundary Value Tests\n");
  printf(1, "-----------------------------\n");

  int pid = fork();
  if(pid == 0) {
    // Test minimum valid capacity
    int result = setgenus(1);
    printf(1, "   setgenus(1) = %d %s\n", result,
           result >= 0 ? "[PASS]" : "[FAIL]");
    printf(1, "   genus=%d, capacity=%d\n", getgenus(), getcapacity());
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0) {
    // Test maximum capacity
    int result = setgenus(90);
    printf(1, "   setgenus(90) = %d %s\n", result,
           result >= 0 ? "[PASS]" : "[FAIL]");
    printf(1, "   genus=%d, capacity=%d\n", getgenus(), getcapacity());
    exit();
  }
  wait();

  printf(1, "\n");
}

void
test_capacity_accounting(void)
{
  printf(1, "TEST 3: Capacity Accounting\n");
  printf(1, "----------------------------\n");
  printf(1, "   Testing repeated allocation and deallocation...\n");

  // Repeatedly allocate and free to check for leaks
  int i;
  for(i = 0; i < 5; i++) {
    int pid = fork();
    if(pid == 0) {
      int result = setgenus(20);
      printf(1, "   [Iteration %d] setgenus(20) = %d %s\n", i, result,
             result >= 0 ? "[PASS]" : "[FAIL]");
      exit();
    }
    wait();
  }

  // Test exact boundary: fill to exactly 90
  printf(1, "\n   Testing exact capacity limit (90):\n");
  int pids[3];

  for(i = 0; i < 3; i++) {
    pids[i] = fork();
    if(pids[i] == 0) {
      sleep(i * 5);
      int result = setgenus(30);
      printf(1, "   [Child %d] setgenus(30) = %d %s\n", i, result,
             result >= 0 ? "[PASS]" : "[FAIL]");
      sleep(100);
      exit();
    }
  }

  sleep(20);

  // Fourth child should fail (total would be 91)
  int pid = fork();
  if(pid == 0) {
    int result = setgenus(1);
    printf(1, "   [Child 3] setgenus(1) = %d %s\n", result,
           result == -1 ? "[PASS - correctly rejected]" : "[FAIL]");
    exit();
  }
  wait();

  // Kill all children to free capacity
  for(i = 0; i < 3; i++) {
    kill(pids[i]);
  }
  for(i = 0; i < 3; i++) {
    wait();
  }

  printf(1, "\n");
}

void
test_nested_inheritance(void)
{
  printf(1, "TEST 4: Nested Inheritance (4 levels)\n");
  printf(1, "---------------------------------------\n");

  int gid = setgenus(25);
  printf(1, "   [Parent] setgenus(25) = %d\n", gid);

  int pid = fork();
  if(pid == 0) {
    // First level child
    int child_genus = getgenus();
    printf(1, "   [Child] genus=%d %s\n", child_genus,
           child_genus == gid ? "[PASS]" : "[FAIL]");

    int pid2 = fork();
    if(pid2 == 0) {
      // Second level grandchild
      int gc_genus = getgenus();
      printf(1, "   [Grandchild] genus=%d %s\n", gc_genus,
             gc_genus == gid ? "[PASS]" : "[FAIL]");

      int pid3 = fork();
      if(pid3 == 0) {
        // Third level great-grandchild
        int ggc_genus = getgenus();
        printf(1, "   [Great-grandchild] genus=%d %s\n", ggc_genus,
               ggc_genus == gid ? "[PASS]" : "[FAIL]");

        // All should fail to set new genus
        int result = setgenus(10);
        printf(1, "   [Great-grandchild] setgenus(10) = %d %s\n", result,
               result == -1 ? "[PASS]" : "[FAIL]");
        exit();
      }
      wait();
      exit();
    }
    wait();
    exit();
  }
  wait();

  printf(1, "\n");
}

void
test_exec_scenarios(void)
{
  printf(1, "TEST 5: Exec Scenarios\n");
  printf(1, "----------------------\n");

  // Test: owner calls exec (should release capacity)
  int pid = fork();
  if(pid == 0) {
    int gid = setgenus(15);
    printf(1, "   [Owner] setgenus(15) = %d\n", gid);
    printf(1, "   [Owner] Calling exec (should release capacity)...\n");

    // exec to a small program that exits immediately
    char *args[] = {"cat", "README", 0};
    exec("cat", args);

    printf(1, "   [Owner] exec failed\n");
    exit();
  }
  wait();

  // Test: inherited child calls exec (should NOT release capacity)
  int gid = setgenus(20);
  printf(1, "   [Parent] setgenus(20) = %d\n", gid);

  pid = fork();
  if(pid == 0) {
    int child_genus = getgenus();
    printf(1, "   [Inherited child] genus=%d (not owner)\n", child_genus);
    printf(1, "   [Inherited child] Calling exec (should NOT release)...\n");

    char *args[] = {"cat", "README", 0};
    exec("cat", args);

    printf(1, "   [Inherited child] exec failed\n");
    exit();
  }
  wait();

  int after_genus = getgenus();
  printf(1, "   [Parent] After child exec, genus=%d %s\n", after_genus,
         after_genus == gid ? "[PASS]" : "[FAIL]");

  printf(1, "\n");
}

void
test_race_conditions(void)
{
  printf(1, "TEST 6: Race Conditions\n");
  printf(1, "-----------------------\n");
  printf(1, "   Testing concurrent setgenus calls near capacity limit...\n");

  // First, fill to 70
  int pids[17];  // 7 + 10 = 17 processes
  int i;

  for(i = 0; i < 7; i++) {
    pids[i] = fork();
    if(pids[i] == 0) {
      sleep(1);
      setgenus(10);
      sleep(200);
      exit();
    }
  }

  sleep(10);

  // Now spawn 10 processes trying to get capacity=5 simultaneously
  // Only 4 should succeed (70 + 4*5 = 90)
  printf(1, "   Spawning 10 processes trying setgenus(5) concurrently...\n");

  for(i = 0; i < 10; i++) {
    pids[7+i] = fork();
    if(pids[7+i] == 0) {
      // All try at the same time (no sleep)
      int result = setgenus(5);
      if(result >= 0) {
        printf(1, "   [Child %d] SUCCESS\n", i);
        sleep(200);
      } else {
        printf(1, "   [Child %d] FAILED (rejected)\n", i);
      }
      exit();
    }
  }

  sleep(20);

  printf(1, "   Expected: 4 success, 6 failures\n");
  printf(1, "   (Race condition test - exact numbers may vary)\n");

  // Kill all
  for(i = 0; i < 17; i++) {
    kill(pids[i]);
  }
  for(i = 0; i < 17; i++) {
    wait();
  }

  printf(1, "\n");
}

void
test_killed_processes(void)
{
  printf(1, "TEST 7: Killed Process Cleanup\n");
  printf(1, "-------------------------------\n");
  printf(1, "   Testing capacity release when owner is killed...\n");

  int pid = fork();
  if(pid == 0) {
    int result = setgenus(30);
    printf(1, "   [Child] setgenus(30) = %d\n", result);
    printf(1, "   [Child] Sleeping...\n");
    sleep(1000);
    exit();
  }

  sleep(10);

  // Kill the child
  printf(1, "   [Parent] Killing child with PID=%d\n", pid);
  kill(pid);
  wait();

  printf(1, "   [Parent] Child killed, capacity should be released\n");

  // Try to allocate 30 again - should succeed if capacity was released
  int pid2 = fork();
  if(pid2 == 0) {
    int result = setgenus(30);
    printf(1, "   [New child] setgenus(30) = %d %s\n", result,
           result >= 0 ? "[PASS - capacity was released]" : "[FAIL]");
    exit();
  }
  wait();

  printf(1, "\n");
}

void
test_zombie_processes(void)
{
  printf(1, "TEST 8: Zombie Process Behavior\n");
  printf(1, "--------------------------------\n");
  printf(1, "   Testing capacity held by zombie processes...\n");

  int pid = fork();
  if(pid == 0) {
    int result = setgenus(40);
    printf(1, "   [Child] setgenus(40) = %d\n", result);
    printf(1, "   [Child] Exiting (will become zombie)...\n");
    exit();
  }

  // Don't wait - child becomes zombie
  sleep(10);

  printf(1, "   [Parent] Child is now zombie (not wait()ed yet)\n");

  // Try to use capacity - should fail because zombie still holds it
  printf(1, "   [Parent] Testing if zombie holds capacity...\n");
  int result_while_zombie = setgenus(60);
  printf(1, "   [Parent] setgenus(60) while zombie exists = %d %s\n",
         result_while_zombie,
         result_while_zombie == -1 ? "[PASS - zombie holds capacity]" :
                                     "[FAIL - zombie should hold capacity]");

  // Now wait for zombie
  printf(1, "   [Parent] Now calling wait() to reap zombie...\n");
  wait();

  // Try again - should succeed now
  printf(1, "   [Parent] Testing after reaping zombie...\n");
  int result_after_reap = setgenus(60);
  printf(1, "   [Parent] setgenus(60) after reap = %d %s\n",
         result_after_reap,
         result_after_reap >= 0 ? "[PASS - capacity released after reap]" : "[FAIL]");

  printf(1, "\n");
}

void
test_genus_id_wraparound(void)
{
  printf(1, "TEST 9: Genus ID Assignment\n");
  printf(1, "---------------------------\n");
  printf(1, "   Testing genus ID uniqueness and assignment...\n");

  int i;

  for(i = 0; i < 10; i++) {
    int pid = fork();
    if(pid == 0) {
      int gid = setgenus(5);
      printf(1, "   [Child %d] genus ID = %d\n", i, gid);
      exit();
    }
    wait();
  }

  printf(1, "   All genus IDs should be unique and incrementing\n");
  printf(1, "   (Note: IDs from previous tests will affect numbering)\n");

  printf(1, "\n");
}
