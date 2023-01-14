# MULTI-PURPOSE PROJECT SCRIPT
# Init variables
CONFIGURE=0;    # Configure CMake
TEST=0          # Run tests
BUILD_TYPE="";  # CMake build type
DIR="";         # Build output directory
BUILD_NOT_ALL=0;
BUILD_SHELL="";
BUILD_TEST="";

# Example usage: ./build.sh release sh -c -t
# Show user help
print_help() {
    echo "Usage: build.sh release|debug [TARGETS] [OPTIONS]...";
    echo "All options (except help) require a valid target (release|debug)!"
    echo "Targets (if none set, build all):";
    echo "\tshell";
    echo "\tshelltest";
    echo "Options:";
    echo "\t-c, --configure                 \t Force (re)generate CMake configuration files";
    echo "\t-h, --help                      \t Display this message";
    echo "\t-t, --tests                     \t Run tests after build";
}

parse_args() {
    arg_num=1;
    parse_mode="";
    # Parse args
    while [ $arg_num -le $# ];
    do
        case "${!arg_num}" in
        "-h" | "--help")
            print_help;
            exit;
            ;;
        "-c" | "--configure")
            CONFIGURE=1;
            ;;
        "-t" | "--tests")
            TEST=1;
            ;;
        "release")
            BUILD_TYPE="RELEASE";
            DIR="release";
            ;;
        "debug")
            BUILD_TYPE="DEBUG";
            DIR="debug";
            ;;
        "shell")
            BUILD_SHELL="shell";
            BUILD_NOT_ALL=1;
            ;;
        "shelltest")
            BUILD_TEST="shelltest";
            BUILD_NOT_ALL=1;
            ;;
        *)
            case "$parse_mode" in
            "r")
                ___UNUSED="${!arg_num}";
                parse_mode="";
                ;;
            *)
                ;;
            esac
            ;;
        esac
        arg_num=$(($arg_num + 1));
    done
    if [ "$parse_mode" != "" ]; then
        echo "Invalid arguments. One or more options are missing values.";
        exit;
    fi
}

do_config() {
    if [ $CONFIGURE -eq 1 ]; then
        echo "Force regenerating build configuration...";
        cmake ../ -DCMAKE_BUILD_TYPE=$BUILD_TYPE;
        echo "Done.";
    fi
}

do_build() {
    echo "Building...";
    if [ $BUILD_NOT_ALL -eq 1 ]; then
        cmake --build . --target "$BUILD_SHELL" "$BUILD_TEST";
    else
        cmake --build .;
    fi
    rc=$?;
    if [ $rc -eq 0 ]; then
        echo "Build finished.";
    else
        echo "Build failed!!!";
        exit;
    fi
}

do_tests() {
    if [ $TEST -eq 1 ]; then
        echo "Running tests...";
        cd test;
        ./shelltest;
        cd ..;
        rc=$?;
        if [ $rc -eq 0 ]; then
            echo "Tests finished.";
        else
            echo "Tests failed!!!";
            exit;
        fi
    else
        echo "Ignoring tests.";
    fi
}

# MAIN BODY STARTS HERE
parse_args $@;
if [ "$BUILD_TYPE" == "" ]; then
    echo "Build type not specified, exiting. Use --help for help.";
    exit 1;
fi

# Zip resources to .XRS
mkdir -p $DIR;
cd $DIR;
# Generate CMake build configuration (opt-in)
do_config;
# Build the project
do_build;
printf '\a';  # Ring ring
# Run tests
do_tests;
cd ..;
# Done!
