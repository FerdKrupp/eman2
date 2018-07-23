:: 1. Run e2version.py and e2speedtest.py
e2version.py
e2speedtest.py

:: 2. Existence tests for data files like images, font files, JSON
python tests\test_EMAN2DIR.py

:: 3. Unit tests
nosetests -vv --exe -m "^test_*" ^
                    -e "^test_image_" ^
                    -e "test_main" ^
                    -e "test_result" ^
                    -e "test_boxing" ^
                    -a \!broken ^
                    %SRC_DIR%\rt\pyem
