
## Complete Setup & Run Guide

### Step 1: Verify Prerequisites

```powershell
# Check MySQL is installed and running
Get-Service -Name MySQL80

# If not running, start it
Start-Service -Name MySQL80

# Verify MySQL is accessible
mysql -u root -p -e "SELECT VERSION();"

# Check Visual Studio 2019 is installed
Test-Path "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional"

# Check Python is installed (for testing)
python --version
```

### Step 2: Setup Database

```powershell
# Navigate to project directory

# Create database and load schema with sample data
mysql -u root -p < database\schema.sql
# Enter password when prompted: Root@123456
```

**Verify database setup:**
```powershell
mysql -u root -p -e "USE sms_service; SELECT Sender, COUNT(*) as patterns FROM sms_patterns GROUP BY Sender;"
```

### Step 3: Configure Application

Edit `config\config.json` and update MySQL credentials:

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "user": "root",
    "password": "Root@123456",  ← Update with your MySQL password
    "database": "sms_service",
    "pool_size": 100,
    "pool_min_size": 50
  },
  "thread_pool": {
    "size": 128
  },
  "network": {
    "host": "0.0.0.0",
    "port": 8080
  }
}
```

### Step 4: Build Application

```powershell
# Run the build script
.\build.bat
```

**If build fails:**
- Verify Visual Studio 2019 path in `build.bat`
- Check MySQL include/lib paths
- Ensure all source files exist

### Step 5: Start the SMS Service

```powershell
# Start the service
.\sms_service.exe
```

### Step 6: Test the Service

**Open a NEW PowerShell terminal** (keep service running in first terminal)

#### Test 1: Single Request (Quick Verification)

```powershell
# Navigate to test directory

# Run single request test
.\single_request_test.ps1
```
## Complete End-to-End Flow Summary

```
1. Start MySQL Service
   ↓
2. Create Database & Load Schema (database\schema.sql)
   ↓
3. Configure Application (config\config.json)
   ↓
4. Build Application (build.bat)
   ↓
5. Start SMS Service (sms_service.exe)
   ↓
6. Test Single Request (single_request_test.ps1)
   ↓
7. Run Stress Test (stress_test.py)
   ↓
8. Monitor Performance (30s status updates)
   ↓
9. Stop Service (Ctrl+C)
```

## Testing

```powershell
# Single request
.\test\single_request_test.ps1

# Stress test
python .\test\stress_test.py
```

## Configuration

Edit `config\config.json` - key settings:
- `thread_pool.size`: 128 (HTTP workers)
- `database.pool_size`: 100 (MySQL connections)

## Key Optimizations

1. 128-thread HTTP server
2. Pre-compiled regex with optimize flags
3. Reader-writer lock (std::shared_mutex)
4. Hybrid substring + regex matching
5. Connection pooling with RAII
 
