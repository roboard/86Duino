#include "myhmi.h"

int _ABVAR_1_counter = 0 ;
int _ABVAR_2_i = 0 ;
int _ABVAR_3_j = 0 ;
int _ABVAR_4_val = 0 ;

void pageChange();
void p4WidgetUpdate();
void p5WidgetUpdate();

void setup()
{
  randomSeed( analogRead(A0) );
  Hmi.begin();

}

void loop()
{
  BEGIN_HMI_EVENT_PROC
  {
    pageChange();
    p4WidgetUpdate();
    p5WidgetUpdate();
  }
  END_HMI_EVENT_PROC
}

void pageChange()
{
  if (Hmi.buttonClicked(p1b1))
  {
    Hmi.gotoPage(p7);
  }
  if (Hmi.buttonClicked(p1b2))
  {
    Hmi.gotoPage(p2);
  }
  if (Hmi.buttonClicked(p2b1))
  {
    Hmi.gotoPage(p1);
  }
  if (Hmi.buttonClicked(p2b2))
  {
    Hmi.gotoPage(p3);
  }
  if (Hmi.buttonClicked(p3b1))
  {
    Hmi.gotoPage(p2);
  }
  if (Hmi.buttonClicked(p3b2))
  {
    Hmi.gotoPage(p4);
  }
  if (Hmi.buttonClicked(p4b1))
  {
    Hmi.gotoPage(p3);
  }
  if (Hmi.buttonClicked(p4b2))
  {
    Hmi.gotoPage(p5);
  }
  if (Hmi.buttonClicked(p5b1))
  {
    Hmi.gotoPage(p4);
  }
  if (Hmi.buttonClicked(p5b2))
  {
    Hmi.gotoPage(p6);
  }
  if (Hmi.buttonClicked(p6b1))
  {
    Hmi.gotoPage(p5);
  }
  if (Hmi.buttonClicked(p6b2))
  {
    Hmi.gotoPage(p7);
  }
  if (Hmi.buttonClicked(p7b1))
  {
    Hmi.gotoPage(p6);
  }
  if (Hmi.buttonClicked(p7b2))
  {
    Hmi.gotoPage(p1);
  }
}

void p4WidgetUpdate()
{
  if (Hmi.pageEnter(p4))
  {
    _ABVAR_1_counter = 0 ;
  }
  if (Hmi.pageTimerEventTriggered(p4))
  {
    _ABVAR_1_counter = ( _ABVAR_1_counter + 1 ) ;
    if (( ( _ABVAR_1_counter ) > ( 100 ) ))
    {
      _ABVAR_1_counter = -100 ;
    }
    Hmi.setBarValue(p4bar2,abs( _ABVAR_1_counter ));
    Hmi.setBarValue(p4bar3,( 100 - abs( _ABVAR_1_counter ) ));
    Hmi.setImageBarValue(p4ibar1,( abs( _ABVAR_1_counter ) / 5 ));
    Hmi.setArcBarValue(p4abar1,abs( _ABVAR_1_counter ));
    Hmi.setArcBarValue(p4abar2,( 100 - abs( _ABVAR_1_counter ) ));
    Hmi.setGaugeValue(p4ga1,abs( _ABVAR_1_counter ),1);
    Hmi.setGaugeValue(p4ga1,( 100 - abs( _ABVAR_1_counter ) ),2);
    Hmi.setImageGaugeValue(p4iga1,abs( _ABVAR_1_counter ));
    Hmi.setLedBright(p4led1,abs( _ABVAR_1_counter ));
    Hmi.setLedBright(p4led2,( 100 - abs( _ABVAR_1_counter ) ));
    Hmi.setLedBright(p4led3,abs( _ABVAR_1_counter ));
    Hmi.setLedBright(p4led4,( 100 - abs( _ABVAR_1_counter ) ));
  }
}

void p5WidgetUpdate()
{
  if (Hmi.pageEnter(p5))
  {
    _ABVAR_1_counter = 0 ;
    for(_ABVAR_2_i = 1; 
1<=4?_ABVAR_2_i <= 4:_ABVAR_2_i >= 4; 
1<=4?_ABVAR_2_i = _ABVAR_2_i + 1:_ABVAR_2_i = _ABVAR_2_i - 1)
    {
      for(_ABVAR_3_j = 1; 
1<=4?_ABVAR_3_j <= 4:_ABVAR_3_j >= 4; 
1<=4?_ABVAR_3_j = _ABVAR_3_j + 1:_ABVAR_3_j = _ABVAR_3_j - 1)
      {
        if (( ( _ABVAR_2_i ) == ( _ABVAR_3_j ) ))
        {
          Hmi.setTableCellText(p5tbl1,_ABVAR_2_i,_ABVAR_3_j,"NoData");
        }
        else
        {
          _ABVAR_4_val = 	random( 1 , 100 ) ;
          Hmi.setTableCellValue(p5tbl1,_ABVAR_2_i,_ABVAR_3_j,_ABVAR_4_val);
        }
      }
    }
    for(_ABVAR_2_i = 0; 
0<=10?_ABVAR_2_i <= 10:_ABVAR_2_i >= 10; 
0<=10?_ABVAR_2_i = _ABVAR_2_i + 1:_ABVAR_2_i = _ABVAR_2_i - 1)
    {
      Hmi.setChartValue(p5ch1,1,_ABVAR_2_i,	random( 1 , 100 ));
      Hmi.setChartValue(p5ch1,2,_ABVAR_2_i,	random( 1 , 100 ));
    }
    for(_ABVAR_2_i = Hmi.getTextListViewRangeStart(p5tl1); 
Hmi.getTextListViewRangeStart(p5tl1)<=Hmi.getTextListViewRangeEnd(p5tl1)?_ABVAR_2_i <= Hmi.getTextListViewRangeEnd(p5tl1):_ABVAR_2_i >= Hmi.getTextListViewRangeEnd(p5tl1); 
Hmi.getTextListViewRangeStart(p5tl1)<=Hmi.getTextListViewRangeEnd(p5tl1)?_ABVAR_2_i = _ABVAR_2_i + 1:_ABVAR_2_i = _ABVAR_2_i - 1)
    {
      _ABVAR_4_val = 	random( 1 , 5 ) ;
      if (( ( _ABVAR_4_val ) == ( 1 ) ))
      {
        Hmi.setTextListLine(p5tl1,_ABVAR_2_i,"Dogs go woof. woof! woof! woof!");
      }
      if (( ( _ABVAR_4_val ) == ( 2 ) ))
      {
        Hmi.setTextListLine(p5tl1,_ABVAR_2_i,"Cats go meow. meow~ meow~ meow~");
      }
      if (( ( _ABVAR_4_val ) == ( 3 ) ))
      {
        Hmi.setTextListLine(p5tl1,_ABVAR_2_i,"Cows go moo. moo~ moo~ moo~");
      }
      if (( ( _ABVAR_4_val ) == ( 4 ) ))
      {
        Hmi.setTextListLine(p5tl1,_ABVAR_2_i,"Bees go buzz. buzz! buzz! buzz!");
      }
      if (( ( _ABVAR_4_val ) == ( 5 ) ))
      {
        Hmi.setTextListLine(p5tl1,_ABVAR_2_i,"Ducks go quack. quack! quack! quack!");
      }
    }
  }
  if (Hmi.pageTimerEventTriggered(p5))
  {
    _ABVAR_1_counter = ( ( _ABVAR_1_counter + 10 ) % 360 ) ;
    Hmi.addOscSample(p5osc1, ( sin( ( ( 3.14159 * _ABVAR_1_counter ) / 180 ) ) * 10 ));
  }
}