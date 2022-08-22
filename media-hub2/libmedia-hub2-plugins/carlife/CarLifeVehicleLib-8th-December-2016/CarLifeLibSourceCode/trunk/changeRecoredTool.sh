#Automatically generate modification history
pre_date="@8th-November-2016@"
pre_version="@V1.2.4@"

cur_date="@8th-December-2016@"
cur_version="@V1.2.4@"


cur_dir=$(pwd)

cpp_files=$(find $cur_dir | grep "\.cpp")
h_files=$(find $cur_dir | grep "\.h")

#echo $cpp_files
#echo $h_files

all_files=$cpp_files
all_files+=" "
all_files+=$h_files
echo $all_files

for file in $all_files
	do
		echo $file
		#echo $pre_date
		#echo $cur_date

		sed -i "s/$pre_date/$cur_date/g" $file
		sed -i "s/$pre_version/$cur_version/g" $file
	done


























