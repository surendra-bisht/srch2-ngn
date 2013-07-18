cd exact_A1
echo '----do exact_A1 test--------------'
python exact_A1.py queriesAndResults.txt

cd ..
cd fuzzy_A1
echo '----do fuzzy_A1 test--------------'
python fuzzy_A1.py queriesAndResults.txt

cd ..
cd exact_M1
echo '----do exact_M1 test--------------'
python exact_M1.py queriesAndResults.txt

cd ..
cd fuzzy_M1
echo '----do fuzzy_M1 test--------------'
python fuzzy_M1.py queriesAndResults.txt

cd ..
cd exact_Attribute_Based_Search
echo '----do exact_Attribute_Based_Search test--------------'
python exact_Attribute_Based_Search.py queriesAndResults.txt

cd ..
cd fuzzy_Attribute_Based_Search
echo '----do fuzzy_Attribute_Based_Search test--------------'
python fuzzy_Attribute_Based_Search.py queriesAndResults.txt

cd ..
cd exact_Attribute_Based_Search_Geo
echo '----do exact_Attribute_Based_Search_Geo test--------------'
python exact_Attribute_Based_Search_Geo.py queriesAndResults.txt

cd ..
cd fuzzy_Attribute_Based_Search_Geo
echo '----do fuzzy_Attribute_Based_Search_Geo test--------------'
python fuzzy_Attribute_Based_Search_Geo.py queriesAndResults.txt

cd ..
cd geo
echo '----do geo test--------------'
python geo.py queriesAndResults.txt

cd ..
cd top_k
echo '----do top_k test--------------'
python test_srch2_top_k.py food 10 20

cd ..
cd tests_used_for_statemedia
echo '----do tests_used_for_statemedia--------------'
sh autotest.sh
