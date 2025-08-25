@echo off
echo ==========================================
echo  Testing Multi-Series Index Fix
echo ==========================================

echo Testing with single column data...
start release\txtplotter.exe "sampledata\simple_line.txt"
timeout /t 3 >nul

echo.
echo Testing with multi-column data...
start release\txtplotter.exe "sampledata\multi_column.txt"
timeout /t 3 >nul

echo.
echo Testing with scientific data...
start release\txtplotter.exe "sampledata\scientific_data.txt"

echo.
echo Test completed! 
echo.
echo Instructions for testing:
echo 1. Load single-column data first
echo 2. Then load multi-column data - multi-column checkbox should appear
echo 3. Check multiple columns and verify multi-series plotting with legend
echo 4. Switch back to single-column data - should not crash
echo 5. The index out of range error should be fixed
echo.
pause