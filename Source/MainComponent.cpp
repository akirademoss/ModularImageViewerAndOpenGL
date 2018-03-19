
/*
==============================================================================

DockableWindows.cpp
Created: 18 March 2018 10:30pm
Author:  Akira DeMoss

==============================================================================
*/


/**
*  In this module, we implement the components defined in previous classes.
*  We add components to a robust modular docking system, provided by github user jcredland.
*/

#include "MainComponent.h"


MainContentComponent::MainContentComponent()
{
	setSize(700, 400);
	addAndMakeVisible(dock);
	addAndMakeVisible(tabDock);
	addAndMakeVisible(advancedDock);
	auto baseColour = Colours::blue.withSaturation(0.4f).withBrightness(0.4f);

	/*
	*	Note that this is simply a proof of concept demo
	*   More work will need to be done to ensure the OpenGLView is working
	*   Properly.
	*/

	//Pointer to be passed into constructor for our FileBrowser
	//This updates our image in our ImageView window when a
	//New file is selected
	ImageView *imgView = new ImageView("Image View");
	


	//Add our File Browser window to our dock
	advancedDock.addComponentToDock(new FileBrowserView("File Browser", *imgView));

	//Add the Image View and OpenGL View windows to the dock
	advancedDock.addComponentToDock(imgView);
	advancedDock.addComponentToDock(new OpenGLView("OpenGL View"));
}

MainContentComponent::~MainContentComponent()
{
}

void MainContentComponent::paint(Graphics& g)
{
	g.fillAll(Colours::black);
}

void MainContentComponent::resized()
{
	auto area = getLocalBounds();
	advancedDock.setBounds(area.reduced(4));
}