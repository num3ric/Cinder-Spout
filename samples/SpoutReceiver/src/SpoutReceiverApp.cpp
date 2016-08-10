/*
				
		Basic Spout receiver for Cinder

		Uses the Spout SDK

		Based on the RotatingBox CINDER example without much modification
		Nothing fancy about this, just the basics.

		Search for "SPOUT" to see what is required

	==========================================================================
	Copyright (C) 2014 Lynn Jarvis.

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	==========================================================================

	11.05.14 - used updated Spout Dll with host fbo option and rgba
	04.06.14 - used updated Spout Dll 04/06 with host fbo option removed
			 - added Update function
			 - moved receiver initialization from Setup to Update for sender detection
	11.07.14 - changed to Spout SDK instead of the dll
	29.09.14 - update with with SDK revision
	12.10.14 - recompiled for release
	03.01.15 - SDK recompile - SpoutPanel detected from registry install path
	04.02.15 - SDK recompile for default DX9 (see SpoutGLDXinterop.h)
	14.02.15 - SDK recompile for default DX11 and auto compatibility detection (see SpoutGLDXinterop.cpp)
	21.05.15 - Added optional SetDX9 call
			 - Recompiled for both DX9 and DX11 for new installer
	26.05.15 - Recompile for revised SpoutPanel registry write of sender name
	01.07.15 - Convert project to VS2012
			 - add a window title
	30.03.16 - Rebuild for 2.005 release - VS2012 /MT

*/

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

// spout
#include "spout.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct SpoutIn {
	SpoutIn( gl::Texture2dRef& texture )
		: mMemorySharedMode{ false }
		, mSize{ app::getWindowSize() }
	{
		// This is a receiver, so the initialization is a little more complex than a sender
		// The receiver will attempt to connect to the name it is sent.
		// Alternatively set the optional bUseActive flag to attempt to connect to the active sender. 
		// If the sender name is not initialized it will attempt to find the active sender
		// If the receiver does not find any senders the initialization will fail
		// and "CreateReceiver" can be called repeatedly until a sender is found.
		// "CreateReceiver" will update the passed name, and dimensions.
		mSenderName[0] = NULL; // the name will be filled when the receiver connects to a sender
		// Optionally set for DirectX 9 instead of default DirectX 11 functions
		// mSpoutReceiver.SetDX9(true);
		if( mSpoutReceiver.CreateReceiver( mSenderName, mSize.x, mSize.y, true ) ) {
			// Optionally test for texture share compatibility
			// GetMemoryShareMode informs us whether Spout initialized for texture share or memory share
			mMemorySharedMode = mSpoutReceiver.GetMemoryShareMode();
			CI_LOG_I( "Memory share: " << mMemorySharedMode );

			resize( texture );
		}
		else {
			throw std::exception( " Failed to initialize receiver." );
		}
	}
	~SpoutIn() {
		mSpoutReceiver.ReleaseReceiver();
	}
	bool getTexture( gl::Texture2dRef& texture ) {
		// Save current global width and height - they will be changed
		// by receivetexture if the sender changes dimensions

		// Try to receive the texture at the current size 
		if( mSpoutReceiver.ReceiveTexture( mSenderName, mSize.x, mSize.y, texture->getId(), texture->getTarget() ) ) {
			//	Width and height are changed for sender change so the local texture has to be resized.
			return ! resize( texture );
		}
		return false;
	}

	ivec2 getSize() {
		return mSize;
	}

	bool resize( gl::Texture2dRef& texture )
	{
		if( texture && mSize == uvec2( texture->getSize() ) )
			return false;

		CI_LOG_I( "Recreated texture with size: " << mSize );
		texture = gl::Texture2d::create( mSize.x, mSize.y, gl::Texture::Format().loadTopDown() );
		return true;
	}

	uvec2 mSize;
	bool mMemorySharedMode;
	char mSenderName[256];			// sender name 
	SpoutReceiver mSpoutReceiver;	// Create a Spout receiver object
};

class SpoutReceiverApp : public App {
public:
	SpoutReceiverApp();
	void draw() override;
	void update() override;
	void mouseDown(MouseEvent event) override;

	gl::TextureRef				mSpoutTexture;
	std::unique_ptr<SpoutIn>	mSpoutIn;
};

SpoutReceiverApp::SpoutReceiverApp()
{
	try {
		mSpoutIn = std::unique_ptr<SpoutIn>( new SpoutIn{ mSpoutTexture } );
	}
	catch( ... ) {
		mSpoutIn = nullptr;
	}
}

void SpoutReceiverApp::update()
{
	if( mSpoutIn ) {
		mSpoutIn->getTexture( mSpoutTexture );

		if( mSpoutTexture->getSize() != app::getWindowSize() ) {
			app::setWindowSize( mSpoutTexture->getSize() );
		}
	}
}

void SpoutReceiverApp::draw()
{
	gl::clear();
	gl::color( Color::white() );
	char txt[256];

	if( mSpoutTexture ) {
		// Otherwise draw the texture and fill the screen
		gl::draw( mSpoutTexture, getWindowBounds() );

		// Show the user what it is receiving
		gl::ScopedBlendAlpha alpha;
		gl::enableAlphaBlending();
		sprintf_s( txt, "Receiving from [%s]", mSpoutIn->mSenderName );
		gl::drawString( txt, vec2( toPixels( 20 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 24 ) ) );
		sprintf_s( txt, "fps : %2.2d", (int)getAverageFps() );
		gl::drawString( txt, vec2( getWindowWidth() - toPixels( 100 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 24 ) ) );
		gl::drawString( "RH click to select a sender", vec2( toPixels( 20 ), getWindowHeight() - toPixels( 40 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 24 ) ) );
	}
	else {
		gl::ScopedBlendAlpha alpha;
		gl::enableAlphaBlending();
		gl::drawString( "No sender detected", vec2( toPixels( 20 ), toPixels( 20 ) ), Color( 1, 1, 1 ), Font( "Verdana", toPixels( 24 ) ) );
	}
}
void SpoutReceiverApp::mouseDown(MouseEvent event)
{
	if( event.isRightDown() && mSpoutIn ) { // Select a sender
		// SpoutPanel.exe must be in the executable path
		mSpoutIn->mSpoutReceiver.SelectSenderPanel(); // DirectX 11 by default
	}
}

void prepareSettings( App::Settings *settings )
{
	settings->setWindowSize( 320, 240 );
}

// This line tells Cinder to actually create the application
CINDER_APP( SpoutReceiverApp, RendererGl, prepareSettings )
