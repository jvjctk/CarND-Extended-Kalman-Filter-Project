#include "FusionEKF.h"
#include <iostream>
#include "Eigen/Dense"
#include "tools.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::cout;
using std::endl;
using std::vector;

/**
 * Constructor.
 */
FusionEKF::FusionEKF() {
  is_initialized_ = false;

  previous_timestamp_ = 0;

  // initializing matrices
  R_laser_ = MatrixXd(2, 2);
  R_radar_ = MatrixXd(3, 3);
  H_laser_ = MatrixXd(2, 4);
  Hj_ = MatrixXd(3, 4);

  //measurement covariance matrix - laser
  R_laser_ << 0.0225, 0,
              0, 0.0225;

  //measurement covariance matrix - radar
  R_radar_ << 0.09, 0, 0,
              0, 0.0009, 0,
              0, 0, 0.09;

  /**
   * TODO: Finish initializing the FusionEKF.
   * TODO: Set the process and measurement noises
   */

  // Measurement transition function
  H_laser_ << 1, 0, 0, 0, 0, 1, 0, 0;

  // State covariance matrix
  MatrixXd P_(4, 4);

  P_ <<  1, 0, 0, 0, 
         0, 1, 0, 0,
         0, 0, 1000, 0,
         0, 0, 0, 1000;
  
// State transition function
  MatrixXd F_(4, 4);

  F_ <<  1, 0, 1, 0,
         0, 1, 0, 1,
         0, 0, 1, 0,
         0, 0, 0, 1;
  // Process noise covariance matrix
  MatrixXd Q_(4, 4);
  // State vector
  VectorXd x_(4);

  // Initialize the Kalman filter attributes
  ekf_.Init(x_, P_, F_, H_laser_, R_laser_, Q_);



}

/**
 * Destructor.
 */
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) {
  /**
   * Initialization
   */
  if (!is_initialized_) {
    
     /**
     * TODO: Initialize the state ekf_.x_ with the first measurement.
     * TODO: Create the covariance matrix.
     * You'll need to convert radar from polar to cartesian coordinates.
     */

    // first measurement
    cout << "EKF: " << endl;
    ekf_.x_ = VectorXd(4);
    ekf_.x_ << 1, 1, 1, 1;

    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
      
      
      // TODO: Convert radar from polar to cartesian coordinates 
      //         and initialize state.

  
      float phi = measurement_pack.raw_measurements_(1);

      // adjusting phi to make between -Pi & Pi
      while (phi > M_PI) {
        phi -= 2.0 * M_PI;
      }
      while (phi < -M_PI) {
        phi += 2.0 * M_PI;
      }

      ekf_.x_ << measurement_pack.raw_measurements_(0) * cos(phi),
          measurement_pack.raw_measurements_(0) * sin(phi),
          measurement_pack.raw_measurements_(2) * cos(phi),
          measurement_pack.raw_measurements_(2) * sin(phi);


    }
    else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
      // TODO: Initialize state.

      ekf_.x_ << measurement_pack.raw_measurements_(0),
          measurement_pack.raw_measurements_(1), 0, 0;

    }

    // done initializing, no need to predict or update

    previous_timestamp_ = measurement_pack.timestamp_;

    is_initialized_ = true;
    return;
  }

  /**
   * Prediction
   */

  /**
   * TODO: Update the state transition matrix F according to the new elapsed time.
   * Time is measured in seconds.
   * TODO: Update the process noise covariance matrix.
   * Use noise_ax = 9 and noise_ay = 9 for your Q matrix.
   */


  float dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0; // in seconds
  previous_timestamp_ = measurement_pack.timestamp_; // updating time stamp

  // updating state transition matrix

  ekf_.F_ << 1, 0, dt, 0, 0, 1, 0, dt, 0, 0, 1, 0, 0, 0, 0, 1;

  // processing covariance matrix

  float noise_ax = 9;
  float noise_ay = 9;
  float dt_2 = dt * dt;
  float dt_3 = dt_2 * dt;
  float dt_4 = dt_3 * dt;

  ekf_.Q_ << dt_4 / 4 * noise_ax, 0, dt_3 / 2 * noise_ax, 0, 0,
      dt_4 / 4 * noise_ay, 0, dt_3 / 2 * noise_ay, dt_3 / 2 * noise_ax, 0,
      dt_2 * noise_ax, 0, 0, dt_3 / 2 * noise_ay, 0, dt_2 * noise_ay;



  ekf_.Predict();

  /**
   * Update
   */

  /**
   * TODO:
   * - Use the sensor type to perform the update step.
   * - Update the state and covariance matrices.
   */


    
  if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
    // TODO: Radar updates

    ekf_.R_ = R_radar_;
    Tools tools;
    Hj_ = tools.CalculateJacobian(ekf_.x_);
    ekf_.H_ = Hj_;
    ekf_.UpdateEKF(measurement_pack.raw_measurements_);


  } else {
    // TODO: Laser updates

    ekf_.R_ = R_laser_;
    ekf_.H_ = H_laser_;
    ekf_.Update(measurement_pack.raw_measurements_);

  }

  // print the output
  cout << "x_ = " << ekf_.x_ << endl;
  cout << "P_ = " << ekf_.P_ << endl;
}
