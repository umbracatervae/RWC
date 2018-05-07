INSTRUCTIONS FOR RUNNING RWGUI.py
>python RWGUI.py <resolution_parameter> <font_parameter>
 <resolution_parameter> and <font_paramater> are both optional arguments but HEAVILY RECOMMENDED
 <resolution_parameter> can be either one of the two options below:
    (1) 720p (which creates a window with a resolution that is slightly < than 720p)
    (2) 1080p (which creates a window with a resolution that is slightly < than 1080p)
 <font_parameter> can either be one of the three options below:
    (1) --native (which assumes you are not using SSH and X11 forwarding)
    (2) --x11 (which assumes you are using SSH and X11 forwarding)
    (3) <value> (any integer value if the above two presets aren't working for you)
    
This GUI script is also run as part of the RWCExecute.csh script if the --gui flag is passed to it.

For more information, ./RWCExecute.csh --help or go RWClustering/docs/ to read the RWGUI manual

---------------------------------
Example usages:

python RWGUI.py 1080p 16
python RWGUI.py 1080p --x11
python RWGUI.py 1080p --native
python RWGUI.py 720p 10
python RWGUI.py 720p --native
python RWGUI.py 720p -x11