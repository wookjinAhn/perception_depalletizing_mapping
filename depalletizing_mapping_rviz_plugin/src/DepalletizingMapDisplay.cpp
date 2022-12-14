//
// Created by wj on 22. 5. 31.
//

// setting about "Displays" panel in rviz

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

#include <tf/transform_listener.h>

#include <rviz/visualization_manager.h>
#include <rviz/properties/color_property.h>
#include <rviz/properties/float_property.h>
#include <rviz/properties/int_property.h>
#include <rviz/frame_manager.h>

#include "depalletizing_mapping_rviz_plugin/DepalletizingMapDisplay.hpp"
#include "depalletizing_mapping_rviz_plugin/DepalletizingMapVisual.hpp"

namespace depalletizing_mapping_rviz_plugin
{
    DepalletizingMapDisplay::DepalletizingMapDisplay()
	{
		mHistoryLengthProperty = new rviz::IntProperty("History Length", 1,
													   "Number of prior measurements to display.",
													   this, SLOT( updateHistoryLength() ));
		mHistoryLengthProperty->setMin(1);
		mHistoryLengthProperty->setMax(100000);

		mAlphaProperty = new rviz::FloatProperty("Alpha", 1.0,
												 "0 is fully transparent, 1.0 is fully opaque.", this,
												 SLOT(updateVisualization()));
		mAlphaProperty->setMin(0.0f);
		mAlphaProperty->setMax(1.0f);

		mColorProperty = new rviz::ColorProperty("Color", QColor(255, 255, 255),
												 "Color to draw the mesh.", this,
												 SLOT(updateVisualization()));
	}

	DepalletizingMapDisplay::~DepalletizingMapDisplay()
	{
	}

	void DepalletizingMapDisplay::onInitialize()
	{
		boost::mutex::scoped_lock lock(mMutex);

		MFDClass::onInitialize();
		updateHistoryLength();
	}

	void DepalletizingMapDisplay::reset()
	{
		boost::mutex::scoped_lock lock(mMutex);

		MFDClass::reset();
		mVisuals.clear();
	}

	void DepalletizingMapDisplay::updateHistoryLength()
	{
		mVisuals.rset_capacity(mHistoryLengthProperty->getInt());
	}

	void DepalletizingMapDisplay::updateVisualization()
	{
		float alpha = mAlphaProperty->getFloat();
		Ogre::ColourValue color = mColorProperty->getOgreColor();

		for(size_t i = 0; i < mVisuals.size(); i++)
		{
			mVisuals[i]->SetColor(color.r, color.g, color.b, alpha);
		}
	}

	void DepalletizingMapDisplay::processMessage(const depalletizing_mapping_msgs::DepalletizingMap::ConstPtr& msg)
	{
		boost::mutex::scoped_lock lock(mMutex);
		Ogre::Quaternion orientation;
		Ogre::Vector3 position;

        std::cout << msg->header.frame_id.c_str() << std::endl;
        std::cout << qPrintable( fixed_frame_ ) << std::endl;
		if(!context_->getFrameManager()->getTransform(msg->header.frame_id,
													  msg->header.stamp,
													  position, orientation))
		{
			ROS_DEBUG( "Error transforming from frame '%s' to frame '%s'", msg->header.frame_id.c_str(), qPrintable( fixed_frame_ ));
			return;
		}

		boost::shared_ptr<DepalletizingMapVisual> visual;

        ROS_INFO("processMessgae boost::shared_ptr<DepalletizingMapVisual> visual");
		if( mVisuals.full() )
		{
			visual = mVisuals.front();
		}
		else
		{
			visual.reset(new DepalletizingMapVisual(context_->getSceneManager(), scene_node_));
		}

		// Now set or update the contents of the chosen visual.
		visual->SetMessage(msg);
		visual->SetFramePosition(position);
		visual->SetFrameOrientation(orientation);

		float alpha = mAlphaProperty->getFloat();
		Ogre::ColourValue color = mColorProperty->getOgreColor();
		visual->SetColor(alpha);

		mVisuals.push_back(visual);
        ROS_INFO("processMessgae End");
	}
} // end namespace depalletizing_mapping_rviz_plugin

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(depalletizing_mapping_rviz_plugin::DepalletizingMapDisplay, rviz::Display)