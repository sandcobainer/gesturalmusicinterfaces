//
// leapmotion.cpp
//
// Max Object for Leap Motion V2 Skeletal tracking in Max
//
// Based on 'aka.leapmotion' by Masayuki Akamatsu - masayukiakamatsu@mac.com
// Additional Support:
// - Full Skeletal Trac-king
// - Background frames
// - Built-in SDK Gestures
//
// author: Jules Françoise
// contact: jules.francoise@ircam.fr
//

extern "C" {
#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
}

#define _USE_MATH_DEFINES // To get definition of M_PI
#include <cmath>
#include <iostream>
#include "Leap.h"

// a macro to mark exported symbols in the code without requiring an external file to define them
#ifdef WIN_VERSION
// note that this is the required syntax on windows regardless of whether the compiler is msvc or gcc
#define T_EXPORT __declspec(dllexport)
#else // MAC_VERSION
// the mac uses the standard gcc syntax, you should also set the -fvisibility=hidden flag to hide the non-marked symbols
#define T_EXPORT __attribute__((visibility("default")))
#endif

////////////////////////// object struct
typedef struct _leapmotion
{
	t_object ob;
	int64_t frame_id_save;
	void *outlet_end;
    void *outlet_gestures;
    void *outlet_fingers;
    void *outlet_hands;
    void *outlet_frame;
    void *outlet_start;
	Leap::Controller *leap;
} t_leapmotion;

///////////////////////// function prototypes
//// standard set
void *leapmotion_new(t_symbol *s, long argc, t_atom *argv);
void leapmotion_free(t_leapmotion *x);
void leapmotion_assist(t_leapmotion *x, void *b, long m, long a, char *s);
void leapmotion_bang(t_leapmotion *x);

//////////////////////// global class pointer variable
void *leapmotion_class;

//////////////////////// Max functions
int T_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("leapmotion", (method)leapmotion_new, (method)leapmotion_free, (long)sizeof(t_leapmotion), 0L /* leave NULL!! */, A_GIMME, 0);
	
    class_addmethod(c, (method)leapmotion_bang, "bang", 0);
    
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)leapmotion_assist, "assist", A_CANT, 0);
	
	class_register(CLASS_BOX, c);
	leapmotion_class = c;
    
	return 0;
}

void leapmotion_assist(t_leapmotion *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { //inlet
		sprintf(s, "bang to cause the frame data output");
	}
	else {	// outlet
		switch (a) {
			case 0:
				sprintf(s, "End Frame");
				break;
            case 1:
				sprintf(s, "Gestures");
				break;
            case 2:
				sprintf(s, "Fingers");
				break;
            case 3:
				sprintf(s, "Hands");
				break;
			case 4:
				sprintf(s, "Frame Info");
				break;
            case 5:
				sprintf(s, "Begin Frame");
				break;
			default:
				break;
		}
	}
}

void leapmotion_free(t_leapmotion *x)
{
	delete (Leap::Controller *)(x->leap);
}

void leapmotion_bang(t_leapmotion *x)
{
	const Leap::Frame frame = x->leap->frame();
	const int64_t frame_id = frame.id();
	
	// ignore the same frame
	if (frame_id == x->frame_id_save)
        return;
	x->frame_id_save = frame_id;
	
    outlet_bang(x->outlet_start);
    
	const Leap::HandList hands = frame.hands();
	const size_t numHands = hands.count();
	
	t_atom frame_data[3];
	atom_setlong(frame_data, frame_id);
	atom_setlong(frame_data+1, frame.timestamp());
	atom_setlong(frame_data+2, numHands);
	outlet_list(x->outlet_frame, NULL, 3, frame_data);
	
	for(size_t i = 0; i < numHands; i++)
	{
		// Hand
		const Leap::Hand &hand = hands[i];
        const int32_t hand_id = hand.id();
		
		const Leap::FingerList &fingers = hand.fingers();
		const size_t numFingers = fingers.extended().count();
		
		t_atom hand_data[11];
		atom_setlong(hand_data, hand_id);
        
        // Hand type
		atom_setsym(hand_data+1, gensym("type"));
        if (hand.isLeft())
            atom_setsym(hand_data+2, gensym("left"));
        else if (hand.isRight())
            atom_setsym(hand_data+2, gensym("right"));
        else
            atom_setsym(hand_data+2, gensym("unknown"));
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        // Confidence
        atom_setsym(hand_data+1, gensym("confidence"));
        atom_setfloat(hand_data+2, hand.confidence());
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
		// Number of fingers
        atom_setsym(hand_data+1, gensym("numfingers"));
        atom_setlong(hand_data+2, numFingers);
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        // Number of tools
        atom_setsym(hand_data+1, gensym("numtools"));
        atom_setlong(hand_data+2, hand.tools().count());
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        // grabStrength
        atom_setsym(hand_data+1, gensym("grabstrength"));
        atom_setfloat(hand_data+2, hand.grabStrength());
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        // pinchStrength
        atom_setsym(hand_data+1, gensym("pinchstrength"));
        atom_setfloat(hand_data+2, hand.pinchStrength());
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        // palmWidth
        atom_setsym(hand_data+1, gensym("palmwidth"));
        atom_setfloat(hand_data+2, hand.palmWidth());
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        // basis
        atom_setsym(hand_data+1, gensym("basis"));
        const Leap::Matrix basis = hand.basis();
        const Leap::Vector xBasis = basis.xBasis;
        const Leap::Vector yBasis = basis.yBasis;
        const Leap::Vector zBasis = basis.zBasis;
        atom_setfloat(hand_data+2, xBasis.x);
        atom_setfloat(hand_data+3, xBasis.y);
        atom_setfloat(hand_data+4, xBasis.z);
        atom_setfloat(hand_data+5, yBasis.x);
        atom_setfloat(hand_data+6, yBasis.y);
        atom_setfloat(hand_data+7, yBasis.z);
        atom_setfloat(hand_data+8, zBasis.x);
        atom_setfloat(hand_data+9, zBasis.y);
        atom_setfloat(hand_data+10, zBasis.z);
        outlet_list(x->outlet_hands, NULL, 11, hand_data);
        
        // direction
        atom_setsym(hand_data+1, gensym("direction"));
        const Leap::Vector direction = hand.direction();
        atom_setfloat(hand_data+2, direction.x);
        atom_setfloat(hand_data+3, direction.y);
        atom_setfloat(hand_data+4, direction.z);
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
        
        // palmPosition
        atom_setsym(hand_data+1, gensym("palmposition"));
        const Leap::Vector palm_position = hand.palmPosition();
        atom_setfloat(hand_data+2, palm_position.x);
        atom_setfloat(hand_data+3, palm_position.y);
        atom_setfloat(hand_data+4, palm_position.z);
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
        
        // palmVelocity
        atom_setsym(hand_data+1, gensym("palmvelocity"));
        const Leap::Vector palm_velocity = hand.palmVelocity();
        atom_setfloat(hand_data+2, palm_velocity.x);
        atom_setfloat(hand_data+3, palm_velocity.y);
        atom_setfloat(hand_data+4, palm_velocity.z);
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
        
        // palmNormal
        atom_setsym(hand_data+1, gensym("palmnormal"));
        const Leap::Vector palm_normal = hand.palmNormal();
        atom_setfloat(hand_data+2, palm_normal.x);
        atom_setfloat(hand_data+3, palm_normal.y);
        atom_setfloat(hand_data+4, palm_normal.z);
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
        
        // wristPosition
        atom_setsym(hand_data+1, gensym("wristposition"));
        const Leap::Vector wrist_position = hand.wristPosition();
        atom_setfloat(hand_data+2, wrist_position.x);
        atom_setfloat(hand_data+3, wrist_position.y);
        atom_setfloat(hand_data+4, wrist_position.z);
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
        
        // elbowPosition
        atom_setsym(hand_data+1, gensym("elbowposition"));
        const Leap::Arm arm = hand.arm();
        const Leap::Vector elbow_position = arm.elbowPosition();
        atom_setfloat(hand_data+2, elbow_position.x);
        atom_setfloat(hand_data+3, elbow_position.y);
        atom_setfloat(hand_data+4, elbow_position.z);
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
        
        // sphereCenter
        atom_setsym(hand_data+1, gensym("spherecenter"));
        const Leap::Vector sphereCenter = hand.sphereCenter();
        atom_setfloat(hand_data+2, sphereCenter.x);
        atom_setfloat(hand_data+3, sphereCenter.y);
        atom_setfloat(hand_data+4, sphereCenter.z);
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
        
        // sphereRadius
        atom_setsym(hand_data+1, gensym("sphereradius"));
        atom_setfloat(hand_data+2, hand.sphereRadius());
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        // Translation
        int motion_history = 1;
        const Leap::Frame frame_hist_motion = x->leap->frame(motion_history);
        
        atom_setsym(hand_data+1, gensym("translationprobability"));
        atom_setfloat(hand_data+2, hand.translationProbability(frame_hist_motion));
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        atom_setsym(hand_data+1, gensym("translation"));
        const Leap::Vector translation = hand.translation(frame_hist_motion);
        atom_setfloat(hand_data+2, translation.x);
        atom_setfloat(hand_data+3, translation.y);
        atom_setfloat(hand_data+4, translation.z);
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
        
        // Scale
        atom_setsym(hand_data+1, gensym("scaleprobability"));
        atom_setfloat(hand_data+2, hand.scaleProbability(frame_hist_motion));
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        atom_setsym(hand_data+1, gensym("scalefactor"));
        atom_setfloat(hand_data+2, hand.scaleFactor(frame_hist_motion));
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
		
        // Rotation
        atom_setsym(hand_data+1, gensym("rotationprobability"));
        atom_setfloat(hand_data+2, hand.rotationProbability(frame_hist_motion));
        outlet_list(x->outlet_hands, NULL, 3, hand_data);
        
        atom_setsym(hand_data+1, gensym("rotationangles"));
        atom_setfloat(hand_data+2, hand.rotationAngle(frame_hist_motion, Leap::Vector::xAxis()));
        atom_setfloat(hand_data+3, hand.rotationAngle(frame_hist_motion, Leap::Vector::yAxis()));
        atom_setfloat(hand_data+4, hand.rotationAngle(frame_hist_motion, Leap::Vector::zAxis()));
        outlet_list(x->outlet_hands, NULL, 5, hand_data);
    }
    
    for(size_t i = 0; i < numHands; i++)
	{
		// Hand
		const Leap::Hand &hand = hands[i];
        const int32_t hand_id = hand.id();
		
		const Leap::FingerList &fingers = hand.fingers();
        const size_t numFingers = fingers.count();
        
		for(size_t j = 0; j < numFingers; j++)
		{
			// Finger
			const Leap::Finger &finger = fingers[j];
			const int32_t finger_id = finger.id();
			
			t_atom finger_data[15];
			atom_setlong(finger_data, hand_id);
            atom_setlong(finger_data+1, finger_id);
            atom_setsym(finger_data+2, gensym("type"));
			switch (finger.type()) {
                case 0:
                    atom_setsym(finger_data+3, gensym("thumb"));
                    break;
                case 1:
                    atom_setsym(finger_data+3, gensym("index"));
                    break;
                case 2:
                    atom_setsym(finger_data+3, gensym("middle"));
                    break;
                case 3:
                    atom_setsym(finger_data+3, gensym("ring"));
                    break;
                case 4:
                    atom_setsym(finger_data+3, gensym("pinky"));
                    break;
                default:
                    atom_setsym(finger_data+3, gensym("unknown"));
                    break;
            }
            outlet_list(x->outlet_fingers, NULL, 4, finger_data);
            
            // Direction
            atom_setsym(finger_data+2, gensym("direction"));
            const Leap::Vector direction = finger.direction();
            atom_setfloat(finger_data+3, direction.x);
            atom_setfloat(finger_data+4, direction.y);
            atom_setfloat(finger_data+5, direction.z);
            outlet_list(x->outlet_fingers, NULL, 6, finger_data);
            
            
            // Width
            atom_setsym(finger_data+2, gensym("width"));
            atom_setfloat(finger_data+3, finger.width());
            outlet_list(x->outlet_fingers, NULL, 4, finger_data);
            
            // Length
            atom_setsym(finger_data+2, gensym("length"));
            atom_setfloat(finger_data+3, finger.length());
            outlet_list(x->outlet_fingers, NULL, 4, finger_data);
            
            // isTool
            atom_setsym(finger_data+2, gensym("istool"));
            atom_setfloat(finger_data+3, finger.isTool());
            outlet_list(x->outlet_fingers, NULL, 4, finger_data);
            
            // Position
            atom_setsym(finger_data+2, gensym("tipposition"));
            const Leap::Vector tip_position = finger.tipPosition();
            atom_setfloat(finger_data+3, tip_position.x);
            atom_setfloat(finger_data+4, tip_position.y);
            atom_setfloat(finger_data+5, tip_position.z);
            outlet_list(x->outlet_fingers, NULL, 6, finger_data);
            
            // Velocity
            atom_setsym(finger_data+2, gensym("tipvelocity"));
            const Leap::Vector tip_velocity = finger.tipVelocity();
            atom_setfloat(finger_data+3, tip_velocity.x);
            atom_setfloat(finger_data+4, tip_velocity.y);
            atom_setfloat(finger_data+5, tip_velocity.z);
            outlet_list(x->outlet_fingers, NULL, 6, finger_data);
            
            // Bones
            // -------------
            
            // Metacarpal
            const Leap::Bone metacarpal = finger.bone(Leap::Bone::TYPE_METACARPAL);
            atom_setsym(finger_data+2, gensym("metacarpal"));
            const Leap::Vector metacarpal_prev_joint = metacarpal.prevJoint();
            atom_setfloat(finger_data+3, metacarpal_prev_joint.x);
            atom_setfloat(finger_data+4, metacarpal_prev_joint.y);
            atom_setfloat(finger_data+5, metacarpal_prev_joint.z);
            const Leap::Vector metacarpal_next_joint = metacarpal.nextJoint();
            atom_setfloat(finger_data+6, metacarpal_next_joint.x);
            atom_setfloat(finger_data+7, metacarpal_next_joint.y);
            atom_setfloat(finger_data+8, metacarpal_next_joint.z);
            const Leap::Vector metacarpal_center = metacarpal.center();
            atom_setfloat(finger_data+9, metacarpal_center.x);
            atom_setfloat(finger_data+10, metacarpal_center.y);
            atom_setfloat(finger_data+11, metacarpal_center.z);
            const Leap::Vector metacarpal_direction = metacarpal.direction();
            atom_setfloat(finger_data+12, metacarpal_direction.x);
            atom_setfloat(finger_data+13, metacarpal_direction.y);
            atom_setfloat(finger_data+14, metacarpal_direction.z);
            outlet_list(x->outlet_fingers, NULL, 15, finger_data);
            
            // Proximal
            const Leap::Bone proximal = finger.bone(Leap::Bone::TYPE_PROXIMAL);
            atom_setsym(finger_data+2, gensym("proximal"));
            const Leap::Vector proximal_prev_joint = proximal.prevJoint();
            atom_setfloat(finger_data+3, proximal_prev_joint.x);
            atom_setfloat(finger_data+4, proximal_prev_joint.y);
            atom_setfloat(finger_data+5, proximal_prev_joint.z);
            const Leap::Vector proximal_next_joint = proximal.nextJoint();
            atom_setfloat(finger_data+6, proximal_next_joint.x);
            atom_setfloat(finger_data+7, proximal_next_joint.y);
            atom_setfloat(finger_data+8, proximal_next_joint.z);
            const Leap::Vector proximal_center = proximal.center();
            atom_setfloat(finger_data+9, proximal_center.x);
            atom_setfloat(finger_data+10, proximal_center.y);
            atom_setfloat(finger_data+11, proximal_center.z);
            const Leap::Vector proximal_direction = proximal.direction();
            atom_setfloat(finger_data+12, proximal_direction.x);
            atom_setfloat(finger_data+13, proximal_direction.y);
            atom_setfloat(finger_data+14, proximal_direction.z);
            outlet_list(x->outlet_fingers, NULL, 15, finger_data);
            
            // Metacarpal
            const Leap::Bone intermediate = finger.bone(Leap::Bone::TYPE_INTERMEDIATE);
            atom_setsym(finger_data+2, gensym("intermediate"));
            const Leap::Vector intermediate_prev_joint = intermediate.prevJoint();
            atom_setfloat(finger_data+3, intermediate_prev_joint.x);
            atom_setfloat(finger_data+4, intermediate_prev_joint.y);
            atom_setfloat(finger_data+5, intermediate_prev_joint.z);
            const Leap::Vector intermediate_next_joint = intermediate.nextJoint();
            atom_setfloat(finger_data+6, intermediate_next_joint.x);
            atom_setfloat(finger_data+7, intermediate_next_joint.y);
            atom_setfloat(finger_data+8, intermediate_next_joint.z);
            const Leap::Vector intermediate_center = intermediate.center();
            atom_setfloat(finger_data+9, intermediate_center.x);
            atom_setfloat(finger_data+10, intermediate_center.y);
            atom_setfloat(finger_data+11, intermediate_center.z);
            const Leap::Vector intermediate_direction = intermediate.direction();
            atom_setfloat(finger_data+12, intermediate_direction.x);
            atom_setfloat(finger_data+13, intermediate_direction.y);
            atom_setfloat(finger_data+14, intermediate_direction.z);
            outlet_list(x->outlet_fingers, NULL, 15, finger_data);
            
            // Distal
            const Leap::Bone distal = finger.bone(Leap::Bone::TYPE_DISTAL);
            atom_setsym(finger_data+2, gensym("distal"));
            const Leap::Vector distal_prev_joint = distal.prevJoint();
            atom_setfloat(finger_data+3, distal_prev_joint.x);
            atom_setfloat(finger_data+4, distal_prev_joint.y);
            atom_setfloat(finger_data+5, distal_prev_joint.z);
            const Leap::Vector distal_next_joint = distal.nextJoint();
            atom_setfloat(finger_data+6, distal_next_joint.x);
            atom_setfloat(finger_data+7, distal_next_joint.y);
            atom_setfloat(finger_data+8, distal_next_joint.z);
            const Leap::Vector distal_center = distal.center();
            atom_setfloat(finger_data+9, distal_center.x);
            atom_setfloat(finger_data+10, distal_center.y);
            atom_setfloat(finger_data+11, distal_center.z);
            const Leap::Vector distal_direction = distal.direction();
            atom_setfloat(finger_data+12, distal_direction.x);
            atom_setfloat(finger_data+13, distal_direction.y);
            atom_setfloat(finger_data+14, distal_direction.z);
            outlet_list(x->outlet_fingers, NULL, 15, finger_data);
            
		}
	}
    
    // Get gestures
    const Leap::GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) {
        const Leap::Gesture gesture = gestures[g];
        t_atom gesture_data[11];
        
        switch (gesture.type()) {
            case Leap::Gesture::TYPE_CIRCLE:
            {
                const Leap::CircleGesture circle = gesture;
                std::string clockwiseness;
                
                if (circle.pointable().direction().angleTo(circle.normal()) <= Leap::PI/4) {
                    clockwiseness = "clockwise";
                } else {
                    clockwiseness = "counterclockwise";
                }
                
                // Calculate angle swept since last frame
                float sweptAngle = 0;
                if (circle.state() != Leap::Gesture::STATE_START) {
                    const Leap::CircleGesture previousUpdate = Leap::CircleGesture(x->leap->frame(1).gesture(circle.id()));
                    sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * Leap::PI;
                }
                
                // Output:
                // gesture circle <id> <state> <duration (s)> <clockwiseness (sym)> <center x> <center y> <radius>
                // <angle (deg)>
                atom_setsym(gesture_data, gensym("circle"));
                atom_setlong(gesture_data+1, gesture.id());
                switch (gesture.state()) {
                    case Leap::Gesture::STATE_START:
                        atom_setsym(gesture_data+2, gensym("start"));
                        break;
                    case Leap::Gesture::STATE_UPDATE:
                        atom_setsym(gesture_data+2, gensym("update"));
                        break;
                    case Leap::Gesture::STATE_STOP:
                        atom_setsym(gesture_data+2, gensym("stop"));
                        break;
                    default:
                        atom_setsym(gesture_data+2, gensym("invalid"));
                        break;
                }
                atom_setfloat(gesture_data+3, gesture.durationSeconds());
                atom_setsym(gesture_data+4, gensym(clockwiseness.c_str()));
                atom_setfloat(gesture_data+5, circle.center()[0]);
                atom_setfloat(gesture_data+6, circle.center()[1]);
                atom_setfloat(gesture_data+7, circle.radius());
                atom_setfloat(gesture_data+8, sweptAngle);
                outlet_list(x->outlet_gestures, gensym("gesture"), 9, gesture_data);
                break;
            }
            case Leap::Gesture::TYPE_SWIPE:
            {
                const Leap::SwipeGesture swipe = gesture;
                
                // Output:
                // gesture swipe <id> <state> <duration (s)> <position x> <position y> <direction x> <direction y>
                // <speed> <startPosition x> <startPosition y>
                atom_setsym(gesture_data, gensym("swipe"));
                atom_setlong(gesture_data+1, gesture.id());
                switch (gesture.state()) {
                    case Leap::Gesture::STATE_START:
                        atom_setsym(gesture_data+2, gensym("start"));
                        break;
                    case Leap::Gesture::STATE_UPDATE:
                        atom_setsym(gesture_data+2, gensym("update"));
                        break;
                    case Leap::Gesture::STATE_STOP:
                        atom_setsym(gesture_data+2, gensym("stop"));
                        break;
                    default:
                        atom_setsym(gesture_data+2, gensym("invalid"));
                        break;
                }
                atom_setfloat(gesture_data+3, gesture.durationSeconds());
                atom_setfloat(gesture_data+4, swipe.position()[0]);
                atom_setfloat(gesture_data+5, swipe.position()[1]);
                atom_setfloat(gesture_data+6, swipe.direction()[0]);
                atom_setfloat(gesture_data+7, swipe.direction()[1]);
                atom_setfloat(gesture_data+8, swipe.speed());
                atom_setfloat(gesture_data+9, swipe.startPosition()[0]);
                atom_setfloat(gesture_data+10, swipe.startPosition()[1]);
                outlet_list(x->outlet_gestures, gensym("gesture"), 11, gesture_data);
                
                break;
            }
            case Leap::Gesture::TYPE_KEY_TAP:
            {
                const Leap::KeyTapGesture keytap = gesture;
                
                // Output:
                // gesture keytap <id> <state> <duration (s)> <position x> <position y> <direction x> <direction y>
                atom_setsym(gesture_data, gensym("keytap"));
                atom_setlong(gesture_data+1, gesture.id());
                switch (gesture.state()) {
                    case Leap::Gesture::STATE_START:
                        atom_setsym(gesture_data+2, gensym("start"));
                        break;
                    case Leap::Gesture::STATE_UPDATE:
                        atom_setsym(gesture_data+2, gensym("update"));
                        break;
                    case Leap::Gesture::STATE_STOP:
                        atom_setsym(gesture_data+2, gensym("stop"));
                        break;
                    default:
                        atom_setsym(gesture_data+2, gensym("invalid"));
                        break;
                }
                atom_setfloat(gesture_data+3, gesture.durationSeconds());
                atom_setfloat(gesture_data+4, keytap.position()[0]);
                atom_setfloat(gesture_data+5, keytap.position()[1]);
                atom_setfloat(gesture_data+6, keytap.direction()[0]);
                atom_setfloat(gesture_data+7, keytap.direction()[1]);
                outlet_list(x->outlet_gestures, gensym("gesture"), 8, gesture_data);
                
                break;
            }
            case Leap::Gesture::TYPE_SCREEN_TAP:
            {
                const Leap::ScreenTapGesture screentap = gesture;
                
                // Output:
                // gesture screentap <id> <state> <duration (s)> <position x> <position y> <direction x> <direction y>
                atom_setsym(gesture_data, gensym("screentap"));
                atom_setlong(gesture_data+1, gesture.id());
                switch (gesture.state()) {
                    case Leap::Gesture::STATE_START:
                        atom_setsym(gesture_data+2, gensym("start"));
                        break;
                    case Leap::Gesture::STATE_UPDATE:
                        atom_setsym(gesture_data+2, gensym("update"));
                        break;
                    case Leap::Gesture::STATE_STOP:
                        atom_setsym(gesture_data+2, gensym("stop"));
                        break;
                    default:
                        atom_setsym(gesture_data+2, gensym("invalid"));
                        break;
                }
                atom_setfloat(gesture_data+3, gesture.durationSeconds());
                atom_setfloat(gesture_data+4, screentap.position()[0]);
                atom_setfloat(gesture_data+5, screentap.position()[1]);
                atom_setfloat(gesture_data+6, screentap.direction()[0]);
                atom_setfloat(gesture_data+7, screentap.direction()[1]);
                outlet_list(x->outlet_gestures, gensym("gesture"), 8, gesture_data);
                
                break;
            }
            default:
                std::cout << "Unknown gesture type." << std::endl;
                break;
        }
    }
    
	outlet_bang(x->outlet_end);
}

void *leapmotion_new(t_symbol *s, long argc, t_atom *argv)
{
	t_leapmotion *x = NULL;
    
	if ((x = (t_leapmotion *)object_alloc((t_class *)leapmotion_class)))
	{
        x->frame_id_save = 0;
        
        x->outlet_start = outlet_new(x, NULL);
        x->outlet_frame = outlet_new(x, NULL);
		x->outlet_hands = outlet_new(x, NULL);
        x->outlet_fingers = outlet_new(x, NULL);
        x->outlet_gestures = outlet_new(x, NULL);
        x->outlet_end = outlet_new(x, NULL);
        
		
		// Create a controller
		x->leap = new Leap::Controller();
        x->leap->enableGesture(Leap::Gesture::TYPE_CIRCLE);
        x->leap->enableGesture(Leap::Gesture::TYPE_KEY_TAP);
        x->leap->enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
        x->leap->enableGesture(Leap::Gesture::TYPE_SWIPE);
        
        // Allow Leap to send data while in background
        x->leap->setPolicyFlags(Leap::Controller::POLICY_BACKGROUND_FRAMES);
	}
	return (x);
}
