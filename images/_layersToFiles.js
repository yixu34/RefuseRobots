
  /////////////////////////////
  // 
  //  This code was written as an example code, modify it your needs !
  //  This code is explained in the tutorials over at [http://www.kirupa.com] look for the 'scripting' tutorials
  //  Written by mlk over at www.kirupaforum.com, post over there for help !
  //
  //  [mlkdesign@online.fr]
  //  july 2004
  //

displayDialogs = DialogModes.NO;

saveOptions = new PNGSaveOptions();

if ((documents.length != 0) && (activeDocument.saved)){
	
	var AD = activeDocument; 
	var CurrentFolder = AD.path;
	var newFolder = AD.name+"_separated"
	var tempFolder = new Folder (CurrentFolder+"/"+newFolder)
	tempFolder.create();

	var tempLayer = AD.artLayers.add();
	var checkArray = new Array(AD.layers.length);

  for(a=1;a<=AD.layers.length;a++){
	var CL = AD.layers[a-1];
	if(!((CL.kind == LayerKind.TEXT)||(CL.kind == LayerKind.NORMAL)||(CL.kind == LayerKind.LayerSet))){
		checkArray[a-1] = 1;
	}
	if(CL.visible == 0){
		checkArray[a-1] = 2;
	}
}


  for(a=1;a<=AD.layers.length;a++){
	AD.layers[a-1].visible = 0;
  }

  for(a=2;a<=AD.layers.length;a++){

	AD.layers[a-2].visible = 0;
	AD.layers[a-1].visible = 1;

	if((checkArray[a-1]!= 1)&&(checkArray[a-1]!= 2)){
		newFile = new File(tempFolder+"/"+"unamed-"+AD.layers[a-1].name+".png");
		AD.saveAs (newFile,saveOptions, true, Extension.LOWERCASE);
	}


  }

  for(a=1;a<=AD.layers.length;a++){
	if(checkArray[a-1] == 2){
		AD.layers[a-1].visible = 0;
	}else{
		AD.layers[a-1].visible = 1;
	}
  }
  AD.layers[0].remove();

}else{
	alert("You either did not save the document or have no document opened !");
}