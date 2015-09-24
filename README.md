intel_index
===========

Brian's code-under-development for the Intelligent Indexing project.
This isn't intended for consumption as much as my own backup. However, if you find a useful implementation in here and need help using it, feel free to contact me.

There are several mostly independent projects residing here:
+EmbAttSpotter: This is the start of a conversion of Almazan et al's supplied Matlab code to C++ for his Embedded Attribute word spotting method. Particularly I want an implementation more applicable to using scenario than a testing scenario.

+SubwordImageAlignment: This is a new idea to align word images we know contain the same n-gram. This will allow us to 1: localize the location of the n-gram, and 2: gather a bunch of aligned n-grams for training.

+brian_handwriting: This has a few things in it:
  -EmbeddedAtt_Almazan: The Matlab code of Almazan, with my
   modified files for testing subword spotting.
   
  -PS_inkball_models_Howe: Howe's supplied Matlab code and 
   my file for testing subword spotting.
   
  -brian_handwriring: Contains the bloated motherload of my 
   C++ word spotting code. Among other things, it contains:
    >An implementation of Aldavert et al's enhanced bag-of-
     visual-words word spotting (from 2015 paper). Still 
     has bugs.
    >An implementation of Liang et al's 2012 synthesis 
     approach to word spotting. Still has bugs.
    
  -py_stuff: Python and Perl scripts for data preprocessing 
   and other small tasks.

+deskewDeslant: A small project that deskews a document image and deslants its text lines.

+nameseperation: Bloated project with graph-cut segementation (including 3D) code.
