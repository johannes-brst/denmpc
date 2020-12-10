/**
 * @file    Ur5e.cpp
 */

#ifndef Ur5e_IKT_H_
#define Ur5e_IKT_H_

#include <Agent.h>
//#include <sensor_msg/JointState.h> //--> TODO
#include <control_msgs/FollowJointTrajectoryGoal.h>
#include <control_msgs/JointTrajectoryControllerState.h>
#include <ros/ros.h>
#include <trajectory_msgs/JointTrajectory.h>
/******************************************************
* Agent                                               *
*******************************************************/

class Ur5e_IKT: public Agent {
  control_msgs::FollowJointTrajectoryGoal subscriber0_old_msg_; //hier evtl Unternachricht "desired" oder: FollowJointTrajectoryGoal
  control_msgs::FollowJointTrajectoryGoal subscriber1_old_msg_;

public:
  Ur5e_IKT(int id = 0);
  Ur5e_IKT(
    std::string joint_states, // joint_states
    std::string goal, // goal
    std::string follow_joint_trajectory, // /scaled_pos_joint_traj_controller/follow_joint_trajectory
    double* init_x,
    double* init_xdes,
    double* init_u,
    double* init_udes,
    double* init_p,
    double* init_d,
    int id
  );

  void setStateSubscriberRosTopicName(std::string rostopicname) {
    ros_state_subscribers_[0]->shutdown();
    *ros_state_subscribers_[0] = ros_node_.subscribe<control_msgs::FollowJointTrajectoryGoal>(rostopicname, 1, &Ur5e_IKT::subStateCallback, this);
  };

// changed from JointTrajectoryControllerState to JointState
  /*
  Header header

  string[] name
  float64[] position
  float64[] velocity
  float64[] effort

  ursprünglich geometry_msgs/Pose2D.msg:
  float64 x
  float64 y
  float64 theta
  */
  void subStateCallback(const control_msgs::FollowJointTrajectoryGoal::ConstPtr& msg) {
    std::vector<double>tmp(dim_x_, 0);
    //  double dt=(msg->header.stamp.nsec-subscriber0_old_msg_.header.stamp.nsec)*1.0e-9;
    //  if(dt>0){
    // --> hier die 6 joint states? wird velocity evtl auch benötigt?
    int ind = 0;
    tmp[0] = msg->trajectory.points[ind].positions[0];
    tmp[1] = msg->trajectory.points[ind].positions[1];
    tmp[2] = msg->trajectory.points[ind].positions[2];
    tmp[3] = msg->trajectory.points[ind].positions[3];
    tmp[4] = msg->trajectory.points[ind].positions[4];
    tmp[5] = msg->trajectory.points[ind].positions[5];
    this->setState(tmp);
    subscriber0_old_msg_ = *msg;
    // }
  };

  // changed from <ur5e::FollowJointTrajectoryGoal> to <control_msgs::FollowJointTrajectoryGoal>
  void setDesiredStateSubscriberRosTopicName(std::string rostopicname) {
    ros_desired_state_subscribers_[0]->shutdown();
    *ros_desired_state_subscribers_[0] = ros_node_.subscribe<control_msgs::FollowJointTrajectoryGoal>(rostopicname, 1, &Ur5e_IKT::subDesiredStateCallback, this);
  };

  // same according to subStateCallback
  void subDesiredStateCallback(const control_msgs::FollowJointTrajectoryGoal::ConstPtr & msg) {
    std::vector<double> tmp(dim_x_, 0);
    //  double dt=(msg->header.stamp.nsec-subscriber1_old_msg_.header.stamp.nsec)*1.0e-9;
    //  if(dt>0){  
    int ind = 0;
    tmp[0] = msg->trajectory.points[ind].positions[0]; 
    tmp[1] = msg->trajectory.points[ind].positions[1];
    tmp[2] = msg->trajectory.points[ind].positions[2];
    tmp[3] = msg->trajectory.points[ind].positions[3];
    tmp[4] = msg->trajectory.points[ind].positions[4];
    tmp[5] = msg->trajectory.points[ind].positions[5];
    this->setDesiredState(tmp);
    subscriber1_old_msg_ = *msg;
    //  }
  }

  void setPublisherRosTopicName(std::string rostopicname) {
    ros_publishers_[0]->shutdown();
    *ros_publishers_[0] = ros_node_.advertise<control_msgs::FollowJointTrajectoryGoal>(rostopicname, 1); // --> goal
  };

  void rosPublishActuation() {
    control_msgs::FollowJointTrajectoryGoal goal; // --> hier stattdessen control_msgs/FollowJointTrajectory Action

    // First, the joint names, which apply to all waypoints
    goal.trajectory.joint_names.push_back("shoulder_pan_joint");
    goal.trajectory.joint_names.push_back("shoulder_lift_joint");
    goal.trajectory.joint_names.push_back("elbow_joint");
    goal.trajectory.joint_names.push_back("wrist_1_joint");
    goal.trajectory.joint_names.push_back("wrist_2_joint");
    goal.trajectory.joint_names.push_back("wrist_3_joint");

    //joint_angles = a;// <----------- woher kommen die angles?

    goal.trajectory.points.resize(1);

    // First trajectory point
    // Positions
    int ind = 0;
    goal.trajectory.points[ind].positions.resize(6); // ###### hier müssen die joint_anlges rein
    goal.trajectory.points[ind].positions[0] = u_[0];
    goal.trajectory.points[ind].positions[1] = u_[1];
    goal.trajectory.points[ind].positions[2] = u_[2];
    goal.trajectory.points[ind].positions[3] = u_[3];
    goal.trajectory.points[ind].positions[4] = u_[4];
    goal.trajectory.points[ind].positions[5] = u_[5];
    goal.trajectory.points[ind].time_from_start = ros::Duration(5.0);
    goal.trajectory.points[ind].velocities.resize(6);
    for (size_t j = 0; j < 8; ++j)
    {
      goal.trajectory.points[ind].velocities[j] = 0.5;//1.0;
    }

    

    //trajectory_msgs::JointTrajectoryPoint point(positions = joint_angles, velocities = [0] * 6, time_from_start = roscpp.Duration(5.0));
    //trajectory_msgs::JointTrajectoryPoint points[1] = { };
    //points[0] = point;

    //goal.trajectory.points[0] = points;

    /*msg.linear.x = u_[0];
    msg.linear.y = 0;
    msg.linear.z = 0;
    msg.angular.x = 0;
    msg.angular.y = 0;
    msg.angular.z = u_[1];*/
    ros_publishers_[0]->publish(goal);
  }

  void f(double  *out, double t, double *x, double *u, double *d, double *p);
  void dfdxlambda(double  *out, double t, double *x, double *u, double *d, double *p, double *lambda);
  void dfdulambda(double  *out, double t, double *x, double *u, double *d, double *p, double *lambda);
  void l(double  *out, double t, double *x, double *u, double *p, double *xdes, double *udes);
  void dldx(double  *out, double t, double *x, double *u, double *p, double *xdes, double *udes);
  void dldu(double  *out, double t, double *x, double *u, double *p, double *xdes, double *udes);
  void v(double  *out, double t, double *x, double *p, double *xdes);
  void dvdx(double  *out, double t, double *x, double *p, double *xdes);
  void ci(double *out, double t, double *x, double *u, double *p, double *pc,  double *xdes, double *udes);
  void dcidxmui(double *out, double t, double *x, double *u, double *p, double *pc,  double *xdes, double *udes, double *mui);
  void dcidumui(double *out, double t, double *x, double *u, double *p, double *pc,  double *xdes, double *udes, double *mui);
  void cia(double  *out, double t, double *x, double *u, double *p, double *pc,  double *xdes, double *udes, double *slack);
  void dciadxmui(double  *out, double t, double *x, double *u, double *p, double *pc,  double *xdes, double *udes, double *mui, double *slack);
  void dciadumui(double  *out, double t, double *x, double *u, double *p, double *pc,  double *xdes, double *udes, double *mu, double *slack);
  void dciadamui(double  *out, double t, double *x, double *u, double *p, double *pc,  double *xdes, double *udes, double *mui, double *slack);
};
#endif

/*
***********
TODO:
- Nachrichten anpassen
- Was genau kommt in den vector bei initialDesiredState? Wohin geht er?
-



***********
turtle topics:
/turtle1/cmd_vel
/turtle1/color_sensor
/turtle1/desiredpose
/turtle1/pose

**********

cpn@cpn-pc:~/catkin_ws/src/denmpc/src/user_scenarios$ rostopic echo  /scaled_pos_int_traj_controller/follow_joint_trajectory/goal
header:
  seq: 4
  stamp:
    secs: 1603373486
    nsecs:  52125930
  frame_id: ''
goal_id:
  stamp:
    secs: 1603373486
    nsecs:  52063941
  id: "/test_move_30558_1603373287058-4-1603373486.052"
goal:
  trajectory:
    header:
      seq: 0
      stamp:
        secs: 0
        nsecs:         0
      frame_id: ''
    joint_names: [shoulder_pan_joint, shoulder_lift_joint, elbow_joint, wrist_1_joint, wrist_2_joint,
  wrist_3_joint]
    points:
      -
        positions: [1.57, -1.57, 1.57, -1.57, 1.57, 1.57]
        velocities: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        accelerations: []
        effort: []
        time_from_start:
          secs: 1
          nsecs:         0
  path_tolerance: []
  goal_tolerance: []
  goal_time_tolerance:
    secs: 0
    nsecs:         0

---

header:
  seq: 248912
  stamp:
    secs: 1603351798
    nsecs: 902979394
  frame_id: ''
joint_names: [shoulder_pan_joint, shoulder_lift_joint, elbow_joint, wrist_1_joint, wrist_2_joint,
  wrist_3_joint]
desired:
  positions: [1.5699999999999994, -1.6336281798667993, 1.5700000000087093, -1.5700000000000007, 1.5699999999999994, 1.5700000000000007]
  velocities: [6.661338147750939e-16, -6.661338147750939e-16, -9.529488309567569e-12, 6.661338147750939e-16, 6.661338147750939e-16, -6.661338147750939e-16]
  accelerations: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
  effort: []
  time_from_start:
    secs: 0
    nsecs:         0
actual:
  positions: [1.5699896812438965, -1.6336475811400355, 1.5700114409076136, -1.5699876111796875, 1.5700187683105469, 1.5699968338012695]
  velocities: [0.0, 0.0, -0.0, -0.0, -0.0, 0.0]
  accelerations: []
  effort: []
  time_from_start:
    secs: 0
    nsecs:         0
error:
  positions: [1.0318756102911664e-05, 1.9401273235963856e-05, -1.1440898903813945e-05, -1.238882031273647e-05, -1.876831054747896e-05, 3.166198730752967e-06]
  velocities: [6.661338147750939e-16, -6.661338147750939e-16, -9.529488309567569e-12, 6.661338147750939e-16, 6.661338147750939e-16, -6.661338147750939e-16]
  accelerations: []
  effort: []
  time_from_start:
    secs: 0
    nsecs:         0
---

#############################################
JointState Message:

header:
  seq: 34166
  stamp:
    secs: 1605782735
    nsecs: 587842425
  frame_id: ''
name: [elbow_joint, shoulder_lift_joint, shoulder_pan_joint, wrist_1_joint, wrist_2_joint,
  wrist_3_joint]
position: [0.05162129717625401, -1.5699112355563507, 1.5700590000000023, -1.5699750054005426, 1.5699804663066734, 1.5699581869770363]
velocity: [0.33441644078073895, -3.958156105454649e-05, -7.734370327354412e-14, 5.607825137327188e-06, -1.9905830798258673e-05, -1.3801152420872458e-05]
effort: [0.09342474606020369, 0.03475279142379313, -0.010680973137309792, 0.06562435510495461, 7.283293095663893e-06, 1.237895673828246e-05]
---


*/
