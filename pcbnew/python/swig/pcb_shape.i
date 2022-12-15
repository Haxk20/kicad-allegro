%ignore EDA_SHAPE::getCenter;
#pragma SWIG nowarn=503

%{
#include <geometry/eda_angle.h>
#include <eda_shape.h>
#include <pcb_shape.h>
%}
%include geometry/eda_angle.h
%include eda_shape.h
%include pcb_shape.h

%extend PCB_SHAPE
{
    EDA_ANGLE GetArcAngleStart()
    {
        EDA_ANGLE startAngle;
        EDA_ANGLE endAngle;
        $self->CalcArcAngles( startAngle, endAngle );
        return startAngle;
    }

    %pythoncode
    %{
    def GetShapeStr(self):
        return self.ShowShape(self.GetShape())
    %}
}

/* Only for compatibility with old python scripts: */
const int S_SEGMENT = (const int)SHAPE_T::SEGMENT;
const int S_RECT = (const int)SHAPE_T::RECT;
const int S_ARC = (const int)SHAPE_T::ARC;
const int S_CIRCLE = (const int)SHAPE_T::CIRCLE;
const int S_POLYGON = (const int)SHAPE_T::POLY;
const int S_CURVE = (const int)SHAPE_T::BEZIER;

%{
/* for compatibility with old python scripts: */
const int S_SEGMENT = (const int)SHAPE_T::SEGMENT;
const int S_RECT = (const int)SHAPE_T::RECT;
const int S_ARC = (const int)SHAPE_T::ARC;
const int S_CIRCLE = (const int)SHAPE_T::CIRCLE;
const int S_POLYGON = (const int)SHAPE_T::POLY;
const int S_CURVE = (const int)SHAPE_T::BEZIER;
%}
