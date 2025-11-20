#!/bin/bash
# SMS Service - Setup Script for Linux/macOS
# Run: chmod +x setup.sh && ./setup.sh

set -e

echo "====================================="
echo "  SMS Service Setup Script"
echo "  High-Performance 500 TPS System"
echo "====================================="
echo ""

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check prerequisites
echo -e "${YELLOW}[1/7] Checking prerequisites...${NC}"

# Check CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: CMake not found. Please install: sudo apt install cmake${NC}"
    exit 1
fi
echo -e "${GREEN}  ✓ CMake found: $(cmake --version | head -n1)${NC}"

# Check MySQL
if ! command -v mysql &> /dev/null; then
    echo -e "${RED}ERROR: MySQL not found. Please install: sudo apt install mysql-server${NC}"
    exit 1
fi
echo -e "${GREEN}  ✓ MySQL found${NC}"

# Check MySQL dev library
if [ -f /usr/include/mysql/mysql.h ] || [ -f /usr/local/include/mysql/mysql.h ]; then
    echo -e "${GREEN}  ✓ MySQL development libraries found${NC}"
else
    echo -e "${YELLOW}  WARNING: MySQL development libraries not found${NC}"
    echo -e "${CYAN}  Install with: sudo apt install libmysqlclient-dev${NC}"
fi

# Check Python
if ! command -v python3 &> /dev/null; then
    echo -e "${YELLOW}  WARNING: Python3 not found. Testing scripts will not work.${NC}"
else
    echo -e "${GREEN}  ✓ Python3 found: $(python3 --version)${NC}"
fi

# Check GCC/Clang
if command -v g++ &> /dev/null; then
    echo -e "${GREEN}  ✓ g++ found: $(g++ --version | head -n1)${NC}"
elif command -v clang++ &> /dev/null; then
    echo -e "${GREEN}  ✓ clang++ found: $(clang++ --version | head -n1)${NC}"
else
    echo -e "${RED}ERROR: No C++ compiler found. Please install: sudo apt install build-essential${NC}"
    exit 1
fi

# Setup third-party libraries
echo ""
echo -e "${YELLOW}[2/7] Setting up third-party libraries...${NC}"

THIRD_PARTY_DIR="$PROJECT_ROOT/third_party"
mkdir -p "$THIRD_PARTY_DIR"

# Clone PugiXML
if [ -d "$THIRD_PARTY_DIR/pugixml" ]; then
    echo -e "${GREEN}  ✓ PugiXML already exists${NC}"
else
    echo -e "${CYAN}  Cloning PugiXML...${NC}"
    cd "$THIRD_PARTY_DIR"
    git clone https://github.com/zeux/pugixml.git
    if [ -d "pugixml" ]; then
        echo -e "${GREEN}  ✓ PugiXML cloned successfully${NC}"
    else
        echo -e "${RED}  ERROR: Failed to clone PugiXML${NC}"
        exit 1
    fi
    cd "$PROJECT_ROOT"
fi

# Clone nlohmann/json
if [ -d "$THIRD_PARTY_DIR/json" ]; then
    echo -e "${GREEN}  ✓ nlohmann/json already exists${NC}"
else
    echo -e "${CYAN}  Cloning nlohmann/json...${NC}"
    cd "$THIRD_PARTY_DIR"
    git clone https://github.com/nlohmann/json.git
    if [ -d "json" ]; then
        echo -e "${GREEN}  ✓ nlohmann/json cloned successfully${NC}"
    else
        echo -e "${RED}  ERROR: Failed to clone nlohmann/json${NC}"
        exit 1
    fi
    cd "$PROJECT_ROOT"
fi

# Configure database
echo ""
echo -e "${YELLOW}[3/7] Configuring database...${NC}"
echo -e "${CYAN}  Please enter your MySQL root password when prompted.${NC}"

if mysql -u root -p < "$PROJECT_ROOT/database/schema.sql"; then
    echo -e "${GREEN}  ✓ Database setup completed${NC}"
else
    echo -e "${YELLOW}  WARNING: Database setup may have failed. Please run manually if needed:${NC}"
    echo -e "${CYAN}  mysql -u root -p < database/schema.sql${NC}"
fi

# Update configuration
echo ""
echo -e "${YELLOW}[4/7] Configuration...${NC}"

CONFIG_FILE="$PROJECT_ROOT/config/config.json"
if [ -f "$CONFIG_FILE" ]; then
    echo -e "${GREEN}  Configuration file exists at: config/config.json${NC}"
    echo -e "${CYAN}  Please update the database password in config/config.json${NC}"
else
    echo -e "${RED}  ERROR: Configuration file not found!${NC}"
    exit 1
fi

# Build project
echo ""
echo -e "${YELLOW}[5/7] Building project...${NC}"

BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo -e "${CYAN}  Running CMake...${NC}"
if cmake .. -DCMAKE_BUILD_TYPE=Release; then
    echo -e "${GREEN}  ✓ CMake configuration successful${NC}"
    
    echo -e "${CYAN}  Building...${NC}"
    CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    if make -j$CORES; then
        echo -e "${GREEN}  ✓ Build successful${NC}"
    else
        echo -e "${RED}  ERROR: Build failed${NC}"
        exit 1
    fi
else
    echo -e "${RED}  ERROR: CMake configuration failed${NC}"
    exit 1
fi

cd "$PROJECT_ROOT"

# Copy configuration
echo ""
echo -e "${YELLOW}[6/7] Copying configuration...${NC}"

mkdir -p "$BUILD_DIR/config"
cp "$CONFIG_FILE" "$BUILD_DIR/config/"
echo -e "${GREEN}  ✓ Configuration copied to build directory${NC}"

# Set permissions
chmod +x "$BUILD_DIR/sms_service"

# Summary
echo ""
echo -e "${YELLOW}[7/7] Setup complete!${NC}"
echo ""
echo -e "${CYAN}=====================================${NC}"
echo -e "${CYAN}  Next Steps:${NC}"
echo -e "${CYAN}=====================================${NC}"
echo ""
echo -e "${NC}1. Update database password in:${NC}"
echo -e "${YELLOW}   build/config/config.json${NC}"
echo ""
echo -e "${NC}2. Start the server:${NC}"
echo -e "${YELLOW}   cd build${NC}"
echo -e "${YELLOW}   ./sms_service${NC}"
echo ""
echo -e "${NC}3. Test the server (in new terminal):${NC}"
echo -e "${YELLOW}   cd test${NC}"
echo -e "${YELLOW}   python3 test_client.py${NC}"
echo ""
echo -e "${NC}4. Run load test:${NC}"
echo -e "${YELLOW}   cd test${NC}"
echo -e "${YELLOW}   python3 load_test.py${NC}"
echo ""
echo -e "${CYAN}=====================================${NC}"
echo -e "${CYAN}  Documentation:${NC}"
echo -e "${CYAN}=====================================${NC}"
echo ""
echo -e "${NC}  README.md              - Full documentation${NC}"
echo -e "${NC}  BUILD_GUIDE.md         - Build instructions${NC}"
echo -e "${NC}  QUICK_REFERENCE.md     - Command cheat sheet${NC}"
echo -e "${NC}  IMPLEMENTATION_DETAILS.md - Technical details${NC}"
echo ""
echo -e "${NC}Executable location:${NC}"
echo -e "${YELLOW}  build/sms_service${NC}"
echo ""
echo -e "${GREEN}Happy coding! Target: 500 TPS${NC}"
echo ""
