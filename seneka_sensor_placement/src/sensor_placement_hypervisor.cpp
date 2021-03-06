/****************************************************************
 *
 * Copyright (c) 2014
 *
 * Fraunhofer Institute for Manufacturing Engineering
 * and Automation (IPA)
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Project name: SeNeKa
 * ROS stack name: seneka_deployment_strategies
 * ROS package name: seneka_sensor_placement
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Author: Muhammad Bilal Chughtai, email:Muhammad.Chughtai@ipa.fraunhofer.de
 *
 * Date of creation: October 2014
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Fraunhofer Institute for Manufacturing
 *     Engineering and Automation (IPA) nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License LGPL as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License LGPL for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License LGPL along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************/

#include "sensor_placement_hypervisor.h"

sensor_placement_hypervisor::sensor_placement_hypervisor()
{
  //subscribe to "sensor_poses" topic
  nav_path_sub_ = nh_.subscribe(std::string("sensor_poses"), 1, &sensor_placement_hypervisor::navPathSubCB, this);

  //publish on "quanjo_maneuver" topic
  quanjo_maneuver_pub_ = nh_.advertise<seneka_sensor_placement::quanjo_maneuver>(std::string("quanjo_maneuver"), 1);

  //sensor_nodes_pickup service initialization
  ss_pickup_sensor_nodes_ = nh_.advertiseService("pickup_sensor_nodes", &sensor_placement_hypervisor::srvCB_pickup_sensor_nodes, this);
}

sensor_placement_hypervisor::~sensor_placement_hypervisor(){}

// callback function for nav_path_sub_ subscriber
void sensor_placement_hypervisor::navPathSubCB(const nav_msgs::Path new_path)
{
  // create quanjo_maneuver msg for deployment
  seneka_sensor_placement::quanjo_maneuver maneuver_msg;
  maneuver_msg.path = new_path;
  maneuver_msg.payload = maneuver_msg.PAYLOAD_DEPLOY;

  // publish the quanjo_maneuver message
  quanjo_maneuver_pub_.publish(maneuver_msg);
  ROS_INFO_STREAM("quanjo_maneuver message sent with PAYLOAD_DEPLOY" << maneuver_msg);

  //save poses of new_path in deployed_nodes_path_
  for (int i = 0; i < new_path.poses.size(); i++)
  {
    deployed_nodes_path_.poses.push_back(new_path.poses.at(i));
  }
}

// callback function for pickup_sensor_nodes
bool sensor_placement_hypervisor::srvCB_pickup_sensor_nodes(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res)
{
  seneka_sensor_placement::quanjo_maneuver maneuver_msg;

  if (!deployed_nodes_path_.poses.empty())
  {
    // create quanjo_maneuver msg for pickup
    maneuver_msg.path = deployed_nodes_path_;
    maneuver_msg.payload = maneuver_msg.PAYLOAD_PICKUP;

    // publish the quanjo_maneuver message
    quanjo_maneuver_pub_.publish(maneuver_msg);
    ROS_INFO_STREAM("quanjo_maneuver message sent with PAYLOAD_PICKUP" << maneuver_msg);

    // clear path after pickup is completed
    deployed_nodes_path_ = nav_msgs::Path();
  }
  else
    ROS_INFO("Did not find any deployed nodes. 'deployed_nodes_path_.poses[]' is empty!");

  return true;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, std::string("sensor_placement_hypervisor"));

  // create sensor_placement_hypervisor node object
  sensor_placement_hypervisor my_placement_hypervisor;

  //specify 10hz loop rate
  ros::Rate loop_rate(10);

  while (my_placement_hypervisor.nh_.ok())
  {
    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}
