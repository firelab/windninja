wget http://mesowest.utah.edu/data/mesowest_csv.tbl
python ninjastation.py --accepted RAWS --in-file mesowest_csv.tbl --out-file raws.kmz
python ninjastation.py --accepted NWS/FAA --accepted "MARITIME" --in-file mesowest_csv.tbl --out-file nws.kmz
python ninjastation.py --accepted "NWS COOP" --accepted "STATE COOP" --accepted "ITD" --accepted "CARB" --accepted "DRI" --in-file mesowest_csv.tbl --out-file coop.kmz
#python ninjastation.py --accepted "APRSWXNET/CWOP" --in-file mesowest_csv.tbl --out-file cwop.kmz
