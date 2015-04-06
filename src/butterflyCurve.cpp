//
// ==========================================================================
// Copyright 2015 Chi-Chang Chu. All rights reserved.
// www.chichangchu.com
// 
// a custom maya node that creates the butterfly curve.
// The butterfly curve is a transcendental plane curve discovered by Temple H. Fay.
// http://en.wikipedia.org/wiki/Butterfly_curve_%28transcendental%29
//
// ==========================================================================
//

#include <string.h>
#include <maya/MIOStream.h>
#include <math.h>

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>

class butterflyCurve : public MPxNode
{
public:
                        butterflyCurve() {};
    virtual             ~butterflyCurve();

    virtual MStatus     compute( const MPlug& plug, MDataBlock& data );
    static  void*       creator();
    static  MStatus     initialize();

public:
    static  MObject     skipBy;
    static  MObject     thetaMultA;
    static  MObject     thetaMultB;
    static  MObject     thetaMultC;
    static  MObject     timesRound;
    static  MObject     cosinMult;
    static  MObject     sinPower;
    static  MObject     zOffset;
    static  MObject     outputCurves;
    static  MTypeId     id;
};

MTypeId     butterflyCurve::id( 0x8000a );
MObject     butterflyCurve::skipBy;
MObject     butterflyCurve::thetaMultA;
MObject     butterflyCurve::thetaMultB;
MObject     butterflyCurve::thetaMultC;
MObject     butterflyCurve::timesRound;
MObject     butterflyCurve::cosinMult;
MObject     butterflyCurve::sinPower;
MObject     butterflyCurve::zOffset;
MObject     butterflyCurve::outputCurves;

// Error macro: if not successful, print error message and return
// the MStatus instance containing the error code.
// Assumes that "stat" contains the error value
#define PERRORfail(stat,msg) \
                                if (!(stat)) { \
                                    stat.perror((msg)); \
                                    return (stat); \
                                }

butterflyCurve::~butterflyCurve () {}

void* butterflyCurve::creator()
{
    return new butterflyCurve();
}

MStatus butterflyCurve::initialize()
{

    MStatus             stat;

    //attribute Fn.
    MFnNumericAttribute nAttr;
    MFnTypedAttribute   typedAttr;

    //skip draw point by
    skipBy = nAttr.create ("skipBy", "skipBy", MFnNumericData::kInt, 5, &stat);
    PERRORfail(stat, "initialize create skipBy attribute");
    stat = addAttribute( skipBy );
    PERRORfail(stat, "addAttribute(skipBy)");

    //The float of times round
    timesRound = nAttr.create ("timesRound", "timesRound", MFnNumericData::kFloat, 100.0, &stat);
    PERRORfail(stat, "initialize create timesRound attribute");
    CHECK_MSTATUS ( nAttr.setKeyable( true ) );
    stat = addAttribute( timesRound );
    PERRORfail(stat, "addAttribute(timesRound)");

    //the first theta multiplier
    thetaMultA = nAttr.create ("thetaMultA", "thetaMultA", MFnNumericData::kFloat, 1.0, &stat);
    PERRORfail(stat, "initialize create thetaMultA attribute");
    CHECK_MSTATUS ( nAttr.setKeyable( true ) );
    stat = addAttribute( thetaMultA );
    PERRORfail(stat, "addAttribute(thetaMultA)");

    //the second theta multiplier
    thetaMultB = nAttr.create ("thetaMultB", "thetaMultB", MFnNumericData::kFloat, 4.0, &stat);
    PERRORfail(stat, "initialize create thetaMultB attribute");
    CHECK_MSTATUS ( nAttr.setKeyable( true ) );
    stat = addAttribute( thetaMultB );
    PERRORfail(stat, "addAttribute(thetaMultB)");

    //the third theta diviser
    thetaMultC = nAttr.create ("thetaMultC", "thetaMultC", MFnNumericData::kFloat, 12.0, &stat);
    PERRORfail(stat, "initialize create thetaMultC attribute");
    CHECK_MSTATUS ( nAttr.setKeyable( true ) );
    stat = addAttribute( thetaMultC );
    PERRORfail(stat, "addAttribute(thetaMultC)");

    //cosin multiplier    
    cosinMult = nAttr.create ("cosinMult", "cosinMult", MFnNumericData::kFloat, 2.0, &stat);
    PERRORfail(stat, "initialize create cosinMult attribute");
    CHECK_MSTATUS ( nAttr.setKeyable( true ) );
    stat = addAttribute( cosinMult );
    PERRORfail(stat, "addAttribute(cosinMult)");

    //sin power
    sinPower = nAttr.create ("sinPower", "sinPower", MFnNumericData::kInt, 5.0, &stat);
    PERRORfail(stat, "initialize create sinPower attribute");
    CHECK_MSTATUS ( nAttr.setKeyable( true ) );
    stat = addAttribute( sinPower );
    PERRORfail(stat, "addAttribute(sinPower)");

    //z offset
    zOffset = nAttr.create ("zOffset", "zOffset", MFnNumericData::kFloat, 0.0, &stat);
    PERRORfail(stat, "initialize create zOffset attribute");
    CHECK_MSTATUS ( nAttr.setKeyable( true ) );
    stat = addAttribute( zOffset );
    PERRORfail(stat, "addAttribute(zOffset)");
    
    //outputCrves
    outputCurves = typedAttr.create( "outputCurves", "outputCurves", MFnNurbsCurveData::kNurbsCurve, &stat );
    PERRORfail(stat, "initialize create outputCurves attribute");
    CHECK_MSTATUS ( typedAttr.setArray( true ) );
    CHECK_MSTATUS ( typedAttr.setReadable( true ) );
    CHECK_MSTATUS ( typedAttr.setWritable( false ) );
    CHECK_MSTATUS ( typedAttr.setUsesArrayDataBuilder( true ) );
    stat = addAttribute( outputCurves );
    PERRORfail(stat, "addAttribute(outputCurves)");

    //attr affects
    stat = attributeAffects( skipBy, outputCurves );
    PERRORfail(stat, "attributeAffects(skipBy, outputCurves)");
    stat = attributeAffects( timesRound, outputCurves );
    PERRORfail(stat, "attributeAffects(timesRound, outputCurves)");
    stat = attributeAffects( thetaMultA, outputCurves );
    PERRORfail(stat, "attributeAffects(thetaMultA, outputCurves)");
    stat = attributeAffects( thetaMultB, outputCurves );
    PERRORfail(stat, "attributeAffects(thetaMultB, outputCurves)");
    stat = attributeAffects( thetaMultC, outputCurves );
    PERRORfail(stat, "attributeAffects(thetaMultC, outputCurves)");
    stat = attributeAffects( cosinMult, outputCurves );
    PERRORfail(stat, "attributeAffects(cosinMult, outputCurves)");
    stat = attributeAffects( sinPower, outputCurves );
    PERRORfail(stat, "attributeAffects(sinPower, outputCurves)");
    stat = attributeAffects( zOffset, outputCurves );
    PERRORfail(stat, "attributeAffects(zOffset, outputCurves)");

    return stat;
}

MStatus butterflyCurve::compute( const MPlug& plug, MDataBlock& data )
{
    MStatus stat;

    if ( plug == outputCurves )
    {
        MDataHandle skipByHandle =  data.inputValue(skipBy, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting skipBy");
        int skipBy = skipByHandle.asInt();

        MDataHandle timesRoundHandle =  data.inputValue(timesRound, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting timesRound");
        float _revolutions = timesRoundHandle.asFloat();

        MDataHandle thetaMultAHandle =  data.inputValue(thetaMultA, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting thetaMultA");
        float _a = thetaMultAHandle.asFloat();

        MDataHandle thetaMultBHandle =  data.inputValue(thetaMultB, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting thetaMultB");
        float _b = thetaMultBHandle.asFloat();

        MDataHandle thetaMultCHandle =  data.inputValue(thetaMultC, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting thetaMultC");
        float _c = thetaMultCHandle.asFloat();

        MDataHandle cosinMultHandle =  data.inputValue(cosinMult, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting cosinMult");
        float _A = cosinMultHandle.asFloat();

        MDataHandle sinPowerHandle =  data.inputValue(sinPower, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting sinPower");
        float _B = sinPowerHandle.asInt();

        MDataHandle zOffsetHandle =  data.inputValue(zOffset, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting zOffset");
        float zOffset = zOffsetHandle.asFloat();

        MArrayDataHandle outputArray = data.outputArrayValue(outputCurves, &stat);
        PERRORfail(stat, "butterflyCurve::compute getting output data handle");

        MPointArray cvs;
        MDoubleArray knots;

        float thetaStep        = 0.01;          // Angle increment
        float _rotation        = 0;             // The rotation offset
        float _yOffset         = 0;             // The y-offset
        float _amplitude       = 10;            // Scale of final drawing
        int section_length     = 10;            // float of sections for each line
        float deg2rad          = M_PI/180;      // Factor to convert degs to radians
        int sl = 0;
        float theta = 0;

        //TODO: check /3
        int _steps = _revolutions * M_PI / thetaStep /3;

        //TODO:clip max draw steps
        //int maxSteps =500;

        int skipCounter = 0;
        int knotCounter = 0;

        for (int t = 0; t <= _steps; t++)
        {
            if (skipCounter == skipBy)
            {
                float r = (exp(cos(_a * theta)) - _A * cos(_b * theta) + pow(sin(theta / _c), _B)) * _amplitude;
                float x = r * cos(theta - _rotation);
                float y = r * sin(theta - _rotation) + _yOffset;
                float z = sin(theta)*zOffset;

                //append point
                MPoint p(x, y, z);
                cvs.append(p);

                //append knots
                knots.append((double)knotCounter);

                skipCounter=0;
                knotCounter++;
            }

            skipCounter++;

            sl++;
            theta += thetaStep;
            if (sl == section_length) sl = 0;
        }

          MFnNurbsCurveData curveCreator;
          MObject finalCurve = curveCreator.create();
          MFnNurbsCurve finalCurveFn;
          finalCurveFn.create(cvs,knots,1,MFnNurbsCurve::kOpen,false,false,finalCurve);
          outputArray.jumpToArrayElement(0);
          MDataHandle curveH = outputArray.outputValue();
          curveH.set(finalCurve);

          //cvs.clear();
          //stat = outputArray.setAllClean();
          //PERRORfail(stat, "butterflyCurve::compute cleaning outputCurves");

    } else {
        return MS::kUnknownParameter;
    }
    
    return stat;
}

MStatus initializePlugin( MObject obj )
{
    MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

    CHECK_MSTATUS (plugin.registerNode( "butterflyCurve", butterflyCurve::id,
                                butterflyCurve::creator, butterflyCurve::initialize ) );
    return MS::kSuccess;;
}

MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plugin( obj );

    CHECK_MSTATUS (plugin.deregisterNode( butterflyCurve::id ) );
    
    return MS::kSuccess;;
}


/*
import maya.cmds as mc
#load plugin
mc.loadPlugin("/USERS/chichang/tools/maya/plugins/butterflyCurve/build/src/butterflyCurve.so")
#create curve shape
outputCurve = mc.curve( n="outputCurve", p=[(0, 0, 0), (3, 5, 6), (5, 6, 7), (9, 9, 9)] )
#make node
butterflyNode = mc.createNode("butterflyCurve")
mc.connectAttr(butterflyNode+".outputCurves[0]", outputCurve+".create", f=True)
*/