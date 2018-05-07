#!/bin/csh -f
#FILE: RWCExecute.csh
#AUTHOR: Akshay Nagendra <akshaynag@gatech>
#DESCRIPTION: File to handle execution of the RWClustering application
#RUN ./RWCExecute.csh --help for example usages and details
set i = 2
set ICD = 3
set MCS = 8
set PID = 0
set POD = 1
set ND = 1
set FONT_SIZE = 0
set lawler
set no_sparse
set no_matrix
set gui
set exp
set native
set x11
set resFlag
set OUTDIR
if ($#argv == 0) then
	echo "Must specify input BLIF files to use"
    echo "Try: ./RWCExecute.csh --help for more information"
	exit
endif
set BLIFILE = $argv[1]
if ( !(-r $BLIFILE) && $argv[1] != "--help" ) then
	echo "[ERROR] $BLIFILE does not exist"
	exit
else if ( $argv[1] == "--help" ) then
    echo "FILE: RWCExecute.csh"
    echo "AUTHOR: Akshay Nagendra <akshaynag@gatech.edu>"
    echo "DESCRIPTION: Script to run RWClustering Application"
    echo "USAGE:"
    echo "./RWCExecute.csh <BLIFILE> [optional args]"
    echo "General Arguments:"
    echo "----------"
    echo "--s/--max_cluster_size <value>:    Specify max cluster size as <value>"
    echo "--c/--inter_cluster_delay <value>:    Specify inter cluster delay as <value>"
    echo "--i/--pi_delay <value>:    Specify every primary input delay as <value>"
    echo "--o/--po_delay <value>:    Specify every primary output delay as <value>"
    echo "--n/--node_delay <value>:    Specify every non-IO gate delay as <value>"
    echo "--lawler:    Use Lawler Labeling and Clustering instead of RW"
    echo "--no_sparse:    Use full delay matrix instead of default sparse matrix"
    echo "--no_matrix:    Use on-the-fly delay calculations instead of matrix (runtime increase; memory decrease)"
    echo "NOTE: --no_matrix overrides --no_sparse"
    echo "--outdir/--od    Specifies the directory to place all output files (default is just RWClustering/)"
    echo "GUI ARGUMENTS:"
    echo "----------"
    echo "--gui    Specifies the GUI is desired to be used (requires the user to specify a font size and a resolution preset"
    echo "  You must specifies a font size for the GUI"
    echo "  Available Options: --native, --x11, or -fs <value> where <value> is an integer specifying the font size"
    echo "  --native means you are running on a local UNIX machie"
    echo "  --x11 means you are running on a UNIX machine which you have ssh-ed into with X11 forwarding"
    echo "  You must also specify the resolution like such:"
    echo "  --res 720p    Specifies 720p resolution"
    echo "  --res 1080p    Specifies 1080p resolution"
    echo "    No other resolutions are supported"
    echo "EXPERIMENTAL ARGUMENTS:"
    echo "----------"
    echo "--exp:    (RW Only) Use experimental cluster-subset based, non-overlap to avoid overlapping clusters (runtime increase)"
    echo "----------"
    echo ""
    echo "EXAMPLE USAGE:"
    echo "    ./RWCExecute.csh example_lecture.blif --s 4 --gui --fs x11 --res 1080p --outdir demo"
    echo "    The above example is great if you are ssh-ed into a UNIX machine, with an x11 server and you have 1080p monitor"
    exit
endif
while ( $i <= $#argv )
	if ($argv[$i] == "--help") then
		echo "FILE: RWCExecute.csh"
        echo "AUTHOR: Akshay Nagendra <akshaynag@gatech.edu>"
        echo "DESCRIPTION: Script to run RWClustering Application"
        echo "USAGE:"
        echo "./RWCExecute.csh <BLIFILE> [optional args]"
        echo "General Arguments:"
        echo "----------"
        echo "--s/--max_cluster_size <value>:    Specify max cluster size as <value>"
        echo "--c/--inter_cluster_delay <value>:    Specify inter cluster delay as <value>"
        echo "--i/--pi_delay <value>:    Specify every primary input delay as <value>"
        echo "--o/--po_delay <value>:    Specify every primary output delay as <value>"
        echo "--n/--node_delay <value>:    Specify every non-IO gate delay as <value>"
        echo "--lawler:    Use Lawler Labeling and Clustering instead of RW"
        echo "--no_sparse:    Use full delay matrix instead of default sparse matrix"
        echo "--no_matrix:    Use on-the-fly delay calculations instead of matrix (runtime increase; memory decrease)"
        echo "NOTE: --no_matrix overrides --no_sparse"
        echo "--outdir/--od    Specifies the directory to place all output files (default is just RWClustering/)"
        echo "GUI ARGUMENTS:"
        echo "----------"
        echo "--gui    Specifies the GUI is desired to be used (requires the user to specify a font size and a resolution preset"
        echo "  You must specifies a font size for the GUI"
        echo "  Available Options: --native, --x11, or -fs <value> where <value> is an integer specifying the font size"
        echo "  --native means you are running on a local UNIX machie"
        echo "  --x11 means you are running on a UNIX machine which you have ssh-ed into with X11 forwarding"
        echo "  You must also specify the resolution like such:"
        echo "  --res 720p    Specifies 720p resolution"
        echo "  --res 1080p    Specifies 1080p resolution"
        echo "    No other resolutions are supported"
        echo "EXPERIMENTAL ARGUMENTS:"
        echo "----------"
        echo "--exp:    (RW Only) Use experimental cluster-subset based, non-overlap to avoid overlapping clusters (runtime increase)"
        echo "----------"
        echo ""
        echo "EXAMPLE USAGE:"
        echo "    ./RWCExecute.csh example_lecture.blif --s 4 --gui --fs x11 --res 1080p --outdir demo"
        echo "    The above example is great if you are ssh-ed into a UNIX machine, with an x11 server and you have 1080p monitor"
        exit
	endif
	if ( $argv[$i] == "--max_cluster_size" || $argv[$i] == "--s") then
		@ i++
		if ( $i > $#argv ) then
			echo "[ERROR] Did not specify max cluster size value"
			exit
		endif
		if ( $argv[$i] == "" ) then
			echo "[ERROR] Did not specify max cluster size value"
			exit
		endif
		@ MCS = $argv[$i]
		@ i++
		continue
	endif
	if ( $argv[$i] == "--inter_cluster_delay" || $argv[$i] == "--c") then
		@ i++
		if ( $i > $#argv ) then
			echo "[ERROR] Did not specify inter cluster delay value"
			exit
		endif
		if ( $argv[$i] == "" ) then
			echo "[ERROR] Did not specify inter cluster delay value"
			exit
		endif
		@ ICD = $argv[$i]
		@ i++
		continue
	endif
	if ( $argv[$i] == "--pi_delay" || $argv[$i] == "--i") then
		@ i++
		if ( $i > $#argv ) then
			echo "[ERROR] Did not specify primary input node delay value"
			exit
		endif
		if ( $argv[$i] == "" ) then
			echo "[ERROR] Did not specify primary input node delay value"
			exit
		endif
		@ PID = $argv[$i]
		@ i++
		continue
	endif
	if ( $argv[$i] == "--po_delay" || $argv[$i] == "--o") then
		@ i++
		if ( $i > $#argv ) then
			echo "[ERROR] Did not specify primary output node delay value"
			exit
		endif
		if ( $argv[$i] == "" ) then
			echo "[ERROR] Did not specify primary output node delay value"
			exit
		endif
		@ POD = $argv[$i]
		@ i++
		continue
	endif
	if ( $argv[$i] == "--node_delay" || $argv[$i] == "--n") then
		@ i++
		if ( $i > $#argv ) then
			echo "[ERROR] Did not specify node delay value"
			exit
		endif
		if ( $argv[$i] == "" ) then
			echo "[ERROR] Did not specify node delay value"
			exit
		endif
		@ ND = $argv[$i]
		@ i++
		continue
	endif
    if ( $argv[$i] == "--outdir" || $argv[$i] == "--od") then
		@ i++
		if ( $i > $#argv ) then
			echo "ERROR: Did not specify output directory"
			exit
		endif
		if ( $argv[$i] == "" ) then
			echo "ERROR: Did not specify output directory"
			exit
		endif
		set OUTDIR = $argv[$i]
		@ i++
		continue
	endif
    if ( $argv[$i] == "--fs") then
		@ i++
		if ( $i > $#argv ) then
			echo "ERROR: Did not specify font size value"
			exit
		endif
		if ( $argv[$i] == "" ) then
			echo "ERROR: Did not specify font size value"
			exit
		endif
		@ FONT_SIZE = $argv[$i]
		@ i++
		continue
	endif
	if ( $argv[$i] == "--lawler" ) then
		set lawler = $argv[$i]
		@ i++
		continue
	endif
	if ( $argv[$i] == "--no_sparse" ) then
		set no_sparse = $argv[$i]
		@ i++
		continue
	endif
	if ( $argv[$i] == "--no_matrix" ) then
		set no_sparse
		set no_matrix = $argv[$i]
		@ i++
		continue
	endif	
	if ( $argv[$i] == "--exp" ) then
		set exp = $argv[$i]
		@ i++
		continue
	endif
	if ( $argv[$i] == "--gui" ) then
		set gui = $argv[$i]
		@ i++
		continue
	endif
    if ( $argv[$i] == "--native" ) then
        set native = $argv[$i]
        @ i++
        continue
    endif
    if ( $argv[$i] == "--x11" ) then
        set x11 = $argv[$i]
        @ i++
        continue
    endif
    if ( $argv[$i] == "--res") then
		@ i++
		if ( $i > $#argv ) then
			echo "ERROR: Did not specify resolution value"
			exit
		endif
		if ( $argv[$i] != "1080p" && $argv[$i] != "720p") then
			echo "ERROR: Did not specify valid resolution"
			exit
		endif
		set resFlag = $argv[$i]
		@ i++
		continue
	endif
	echo "UNSUPPORTED OPTION: $argv[$i]"
	exit	
end
if ( $gui != "" && (($native == "" && $x11 == "" && $FONT_SIZE == 0) || $resFlag == "")) then
    echo "[ERROR] You must specify a font size (either by --native, --x11, or --fs <font size value>) and a resolution (--res 720p or --res 1080p)"
    exit
endif
if ( $gui != "" && $native != "" && $x11 != "" ) then
    echo "[ERROR] You cannot specify both --native or -x11; Pick one"
    exit
endif
if ( $gui != "" && ($native != "" || $x11 != "") && $FONT_SIZE != 0 ) then
    echo "[WARNING] Overiding font preset $native $x11 with --fs $FONT_SIZE"
    set native = ""
    set x11 = ""
endif
if ( $exp != "" && $lawler != "" ) then
    echo "[WARNING] Experimental methods do not support Lawler; Turning off Lawler..."
    set lawler = ""
endif
if ( $gui != "" && $lawler != "" ) then
    echo "[WARNING] RWGUI does not support Lawler at this time; Turning off GUI..."
    set gui = ""
endif
if ( $gui != "" && $exp != "" ) then
    echo "[WARNING] RWGUI does not support experimental clustering at this time; Turning off GUI..."
    set gui = ""
endif

echo "--------------------"
echo "[RWCEXECUTE] RUN PARAMETERS"
echo "--------------------"
echo "BLIF FILE: $BLIFILE"
echo "MAX CLUSTER SIZE: $MCS"
echo "INTER CLUSTER DELAY: $ICD"
echo "PI DELAY: $PID"
echo "PO DELAY: $POD"
echo "NODE DELAY: $ND"
if ( $lawler != "" ) then
	echo "LAWLER MODE: ENABLED"
else
	echo "LAWLER MODE: DISABLED"
endif
if ( $no_sparse == "" && $no_matrix == "") then
	echo "SPARSE MATRIX MODE: ENABLED"
else
	echo "SPARSE MATRIX MODE: DISABLED"
endif
if ( $no_matrix == "") then
	echo "MATRIX MODE: ENABLED"
else
	echo "MATRIX MODE: DISABLED"
endif
if ( $exp != "" ) then
	echo "EXPERIMENTAL NON-OVERLAP CLUSTER MODE: ENABLED"
else
	echo "EXPERIMENTAL NON-OVERLAP CLUSTER MODE: DISABLED"
endif
if ( $gui != "" ) then
	echo "GUI MODE: ENABLED WITH $native$x11$FONT_SIZE $resFlag"
else
	echo "GUI MODE: DISABLED"
endif

#BEGIN RW CLUSTERING EXECUTION
#USER MUST HAVE CALLED CMAKE . AND MAKE
echo "--------------------"
echo "[RWCEXECUTE] CLEANING FILES"
echo "--------------------"
rm output_*
rm Python/input_graph.dmp
if ( $OUTDIR != "" ) then
    echo "--------------------"
    echo "[RWCEXECUTE] SETTING UP OUTPUT DIRECTORY $OUTDIR"
    echo "--------------------"
    if ( -d $OUTDIR ) then
        echo "[WARNING] $OUTDIR already exists; Moving on..."
    else    
        mkdir $OUTDIR
        if ( $status != 0 ) then
            echo "[ERROR] $OUTDIR creation failed; Check your permissions to create folders"
        endif
        echo "$OUTDIR created successfully"
    endif
endif
echo "--------------------"
echo "[RWEXECUTE] RUNNING RWCLUSTERING APPLICATION"
echo "--------------------"
./rw $BLIFILE --max_cluster_size $MCS -inter_cluster_delay $ICD --pi_delay $PID --po_delay $POD --node_delay $ND $lawler $no_sparse $no_matrix $exp $gui
if ( $status != 0 ) then
    echo "--------------------"
    echo "[RWCEXECUTE] EXECUTION STATUS: FAILURE"
    echo "--------------------"
    exit
endif
echo "--------------------"
echo "[RWCEXECUTE] EXECUTION STATUS: PASS"
echo "--------------------"
if ( $OUTDIR != "" ) then
    echo "--------------------"
    echo "[RWCEXECUTE] MOVING OUTPUT FILES TO $OUTDIR"
    echo "--------------------"
    mv output_* $OUTDIR/
    if ( $status != 0 ) then
        echo "[ERROR] Move to $OUTDIR failed"
        exit
    endif
endif
if ( -r "Python/input_graph.dmp" ) then
    echo "--------------------"
    echo "[RWCEXECUTE] RUNNING INTERACTIVE PYTHON GUI"
    echo "--------------------"
    cd Python
	if ( $FONT_SIZE != 0  ) then
    	python RWGUI.py $resFlag $native $x11 $FONT_SIZE
	else
		python RWGUI.py $resFlag $native $x11
	endif
    if ( $status != 0 ) then
        echo "PYTHON GUI FAILURE"
    endif
    exit
endif
if ($gui != "") then
    echo "--------------------"
    echo "[RWCEXECUTE] GUI ABORTED DUE TO SIZE"
    echo "--------------------"
    exit
endif
