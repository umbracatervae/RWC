RWClustering Application
Authors: Akshay Nagendra <akshaynag@gatech.edu>
         Paul Yates <paul.maxyat@gatech.edu>

Description: C++ implementation of the Rajaraman-Wong clustering algorithm designed to cluster gates in a complex input circuit netlist while minimizing the increase in critical delay through the circuit

Instructions for execution:
(1) Unzip RWClustering.zip in your preferred directory as such and go into it:
    >>>unzip RWClustering.zip
    >>>cd RWClustering/
If done correctly you should see multiple BLIF files and directories such as "Include", "src", and such. If you only see "RWClustering/," you may have to "cd RWClustering/" once more
(2) Execute the following commands:
    >>> cmake .
    >>> make
    >>> chmod +x RWCExecute.csh
(3) Copy/Move any .blif files you wish to use into the RWClustering/ directory (it will be the same directory as where the rw executable will be created if running on an UNIX machine)
(4) Execute the RWCExecute.csh script (>>> ./RWCExecute.csh --help to see all the possible options the RWClustering application can be run with)
  (a) Demo execution (if using SSH and X11 forwarding to run on an UNIX machine): ./RWCExecute.csh example_lecture.blif --s 4 --c 3 --gui --x11 --res 1080p
  (b) Demo execution (if running locally on an UNIX machine): ./RWCExecute.csh example_lecture.blif --s 4 --c 3 --gui --native --res 1080p
  (c) If either of the top two presets don't result in a beautiful GUI (font size is too large), you can specify your own font as such:
        ./RWCExecute.csh example_lecture.blif --s 4 --c 3 --gui --fs <INSERT YOUR FONT SIZE VALUE HERE> --res <Either 720p or 1080p>
        
  
Further resources:
  Please consult the docs/ subdirectory for documentation about the project and a manual on how to use the interactive GUI (RWGUI.py)
