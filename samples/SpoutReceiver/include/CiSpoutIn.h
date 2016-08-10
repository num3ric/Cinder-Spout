#pragma once

#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "spout.h"

namespace cinder {

class SpoutIn {
public:
	SpoutIn()
		: mMemorySharedMode{ false }
		, mSize{}
		, mTexture{ nullptr }
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

			resize();
		}
		else {
			throw std::exception( " Failed to initialize receiver." );
		}
	}
	~SpoutIn() {
		mSpoutReceiver.ReleaseReceiver();
	}

	gl::Texture2dRef receiveTexture() {
		// Try to receive the texture at the current size 
		if( mSpoutReceiver.ReceiveTexture( mSenderName, mSize.x, mSize.y, mTexture->getId(), mTexture->getTarget() ) ) {
			//	Width and height are changed for sender change so the local texture has to be resized.
			return resize() ? nullptr : mTexture;
		}
		return nullptr;
	}

	glm::ivec2				getSize() const { return mSize; }
	std::string				getSenderName() const { return mSenderName; }
	SpoutReceiver&			getSpoutReceiver() { return mSpoutReceiver; }
	const SpoutReceiver&	getSpoutReceiver() const { return mSpoutReceiver; }
	bool					isMemoryShareMode() const { return mMemorySharedMode; }
private:
	bool resize()
	{
		if( mTexture && mSize == glm::uvec2( mTexture->getSize() ) )
			return false;

		CI_LOG_I( "Recreated texture with size: " << mSize );
		mTexture = gl::Texture2d::create( mSize.x, mSize.y, gl::Texture::Format().loadTopDown() );
		return true;
	}

	bool				mMemorySharedMode;
	char				mSenderName[256]; // sender name 
	SpoutReceiver		mSpoutReceiver;	// Create a Spout receiver object
	glm::uvec2			mSize;
	gl::TextureRef		mTexture;
};

} // end namespace ci
