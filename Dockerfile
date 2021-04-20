# Change this to a repo controlled by the project, of course
FROM faddat/clarion-env

# Compile and install Clarion
RUN cd /clarion && \
                mkdir build && \
                cd build && \
                cmake -DCMAKE_BUILD_TYPE=Release .. && \
                make -j$(nproc) && \
                ctest -j$(nproc) && \
                mkdir -p clariondata
                
                
# From here, it may be possible to add a second stage that does not contain the build environment.  Not certain it's desirable. 
