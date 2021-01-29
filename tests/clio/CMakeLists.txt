add_executable( clio_tests 
                clio_tests.cpp
                flat_views.cpp
                benchmark.cpp
              )

target_link_libraries( clio_tests clio Catch2 )

