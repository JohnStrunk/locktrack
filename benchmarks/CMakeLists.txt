add_executable(lockbench
               lockbench.c)
target_link_libraries(lockbench
                      locktrack
                      pthread)
