#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "sparser.h"
#include "typelist.h"

TOperatorSignature::TOperatorSignature(const short shortVal, const olxstr &strVal) {
  ShortValue = shortVal;
  StringValue = strVal;
}

TOperatorSignature TOperatorSignature::DefinedFunctions[] = {
                                          TOperatorSignature(aofAdd,  "+"),
                                          TOperatorSignature(aofSub,  "-"),
                                          TOperatorSignature(aofMul,  "*"),
                                          TOperatorSignature(aofDiv,  "/"),
                                          TOperatorSignature(aofExt,  "^"),
                                          TOperatorSignature(aofSin,  "sin"),
                                          TOperatorSignature(aofCos,  "cos"),
                                          TOperatorSignature(aofTan,  "tan"),
                                          TOperatorSignature(aofAsin, "asin"),
                                          TOperatorSignature(aofAcos, "acos"),
                                          TOperatorSignature(aofAtan, "atan"),
                                          TOperatorSignature(aofAbs,  "abs"),
                                          TOperatorSignature(aofAbs,  "%")
                                        };
short TOperatorSignature::DefinedFunctionCount = 13;


TSyntaxParser::TSyntaxParser(IEvaluatorFactory* FactoryInstance, const olxstr& Expression)  {
  EvaluatorFactory = FactoryInstance;
  LogicalOperators.Add("&&", new TtaFactory<IEvaluable, TloAndOperator,IEvaluable>());
  LogicalOperators.Add("||", new TtaFactory<IEvaluable, TloOrOperator, IEvaluable>());
  LogicalOperators.Add("!", new TsaFactory<IEvaluable, TloNotOperator, IEvaluable>());

  ComparisonOperators.Add("==", new TtaFactory<IEvaluable, TcoEOperator, IEvaluator>());
  ComparisonOperators.Add("!=", new TtaFactory<IEvaluable, TcoNEOperator, IEvaluator>());
  ComparisonOperators.Add("<=", new TtaFactory<IEvaluable, TcoLEOperator, IEvaluator>());
  ComparisonOperators.Add(">=", new TtaFactory<IEvaluable, TcoGEOperator, IEvaluator>());
  ComparisonOperators.Add("<", new TtaFactory<IEvaluable, TcoLOperator, IEvaluator>());
  ComparisonOperators.Add(">", new TtaFactory<IEvaluable, TcoGOperator, IEvaluator>());

  Root = SimpleParse( olxstr::DeleteChars(Expression, ' ') );
}
TSyntaxParser::~TSyntaxParser()  {
  for( size_t i=0; i < Evaluables.Count(); i++ )
    delete Evaluables[i];
  for( size_t i=0; i < Evaluators.Count(); i++ )
    delete Evaluators[i];
  for( size_t i=0; i < LogicalOperators.Count(); i++ )
    delete LogicalOperators.GetObject(i);
  for( size_t i=0; i < ComparisonOperators.Count(); i++ )
    delete ComparisonOperators.GetObject(i);
}

IEvaluable* TSyntaxParser::SimpleParse(const olxstr& Exp)  {
  olxstr LeftExp, RightExp, LeftStr, RightStr;
  IEvaluable *LeftCondition = NULL, *RightCondition = NULL;
  IEvaluable *LogicalOperator = NULL;
  TObjectFactory<IEvaluable> *loFactory = NULL, *coFactory=NULL;
  olxstr ComplexExp;
  for( size_t i=0; i < Exp.Length(); i++ )  {
    olxch Char = Exp.CharAt(i);
    if( Char == '(' )  {
      if( ++i >= Exp.Length() )  {
        FErrors.Add( olxstr("Unclosed brackets") );
        break;
      }
      int bc = 1;
      Char = Exp.CharAt(i);
      ComplexExp = EmptyString;
      while( bc != 0 )  {
        if( Char == '(' )  bc++;
        if( Char == ')' )  bc--;
        if( bc )           ComplexExp << Char;
        else               break;
        if( ++i > Exp.Length() )  {
          FErrors.Add( olxstr("Unclosed brackets") );
          break;
        }
        Char = Exp.CharAt(i);
      }
      if( !ComplexExp.IsEmpty() )  {
        if( loFactory )
          RightCondition = SimpleParse(ComplexExp);
        else
          LeftCondition = SimpleParse(ComplexExp);
      }
      if( ++i < Exp.Length() )  Char = Exp.CharAt(i);
      else                      Char = '\0';
    }

    while( (Char <= 'z' && Char >= 'a') ||
           (Char <= 'Z' && Char >= 'A') ||
           (Char <= '9' && Char >= '0') ||
           (Char == '_') || (Char =='.') )
    {
      // put values to different strings
      if( coFactory != NULL )  {
        if( !RightStr.IsEmpty() )  RightStr << Char;
        else                       RightExp << Char;
      }
      else  {
        if( !LeftStr.IsEmpty() )  LeftStr << Char;
        else                      LeftExp << Char;
      }
      if( ++i >= Exp.Length() )  break;
      Char= Exp.CharAt(i);
    }

    // spaces are for readibility only
//    if( Char == ' ' )  continue;

    if( Char == '\'' || Char == '\"' )  {  // string beginning
      olxch StringWrappingChar = Char;
      while( true )  {
        if( ++i >= Exp.Length() )  {
          FErrors.Add( olxstr("Unclose quotation") );
          break;
        }
        Char = Exp.CharAt(i);
        if( Char == StringWrappingChar )  {
          if( coFactory != NULL )  {
            if( !RightStr.IsEmpty() && RightStr[RightStr.Length()-1] == '\\' )  RightStr[RightStr.Length()-1] = Char;
            else  break;
          }
          else  {
            if( !LeftStr.IsEmpty() && LeftStr[LeftStr.Length()-1] == '\\' )  LeftStr[LeftStr.Length()-1] = Char;
            else  break;
          }
        }
        if( coFactory != NULL )  RightStr << Char;
        else             LeftStr << Char;
      }
      if( ++i < Exp.Length() )  Char = Exp.CharAt(i);
      else                      Char = '\0';
    }
 
    // processing comparison operators
    if( coFactory != NULL && (!LeftExp.IsEmpty() || !LeftStr.IsEmpty()) && (!RightExp.IsEmpty() || !RightStr.IsEmpty()) )  {
      IEvaluator *LeftEvaluator = NULL, *RightEvaluator = NULL;
      if( !LeftExp.IsEmpty() )  {
        if( LeftExp.IsNumber() )  {
          LeftEvaluator = new TScalarEvaluator( LeftExp.ToDouble() );
          Evaluators.Add( LeftEvaluator );
        }
        else if( LeftExp.Equalsi("true") || LeftExp.Equalsi("false") )  {
          LeftEvaluator = new TBoolEvaluator( LeftExp.ToBool() );
          Evaluators.Add( LeftEvaluator );
        }
        else  {
          LeftEvaluator = EvaluatorFactory->Evaluator( LeftExp );
          if( LeftEvaluator == NULL )  FErrors.Add( olxstr("Could not find evaluator for: ") << LeftExp);
        }
      }
      else  {
        LeftEvaluator = new TStringEvaluator( LeftStr );
        Evaluators.Add( LeftEvaluator );
      }
      if( !RightExp.IsEmpty() )  {
        if( RightExp.IsNumber() )  {
          RightEvaluator = new TScalarEvaluator( RightExp.ToDouble() );
          Evaluators.Add( RightEvaluator );
        }
        else if( RightExp.Equalsi("true") || !RightExp.Equalsi("false") )  {
          RightEvaluator = new TBoolEvaluator( RightExp.ToBool() );
          Evaluators.Add( RightEvaluator );
        }
        else  {
          RightEvaluator = EvaluatorFactory->Evaluator( RightExp );
          if( RightEvaluator == NULL )  FErrors.Add( olxstr("Could not find evaluator for: ") << RightExp);
        }
      }
      else  {
        RightEvaluator = new TStringEvaluator( RightStr );
        Evaluators.Add( RightEvaluator );
      }
      //RightEvaluator = EvaluatorFactory->NewEvaluator( RightExp.Length() ? RightExp : RightStr );
      TPtrList<IEObject> Args;
      Args.Add( LeftEvaluator );
      Args.Add( RightEvaluator );
      if( loFactory != NULL )  {
        RightCondition = coFactory->NewInstance(&Args);
        Evaluables.Add( RightCondition );
      }
      else  {
        LeftCondition = coFactory->NewInstance(&Args);
        Evaluables.Add( LeftCondition );
      }
      // clean up the values for the next loop
      RightExp  = EmptyString;
      LeftExp  = EmptyString;
      LeftStr  = EmptyString;
      RightStr  = EmptyString;
      coFactory = NULL;
    }

    // process logical operators
    // do not check the left condition - for '!' operator it might be empty or
    // if there is a logical operator on the left (on the right it can be only
    // in the case of brackets)
    if( loFactory && RightCondition )  {
      TPtrList<IEObject> Args;
      if( LogicalOperator != NULL )  {
        Args.Add( LogicalOperator );
        Args.Add( RightCondition );
        LogicalOperator = loFactory->NewInstance(&Args);
      }
      else  {
        Args.Add( LeftCondition );
        Args.Add( RightCondition );
        LogicalOperator = loFactory->NewInstance(&Args);
      }
      Evaluables.Add( LogicalOperator );
      LeftCondition = NULL;
      RightCondition = NULL;
      loFactory = NULL;
    }

    if( coFactory == NULL && (i < Exp.Length()) )  {
      for( size_t j=0; j < ComparisonOperators.Count(); j++ )  {
        size_t index = 0;
        while( (index < ComparisonOperators.GetString(j).Length()) &&
               (ComparisonOperators.GetString(j).CharAt(index) == Char)  )
        {
          if( (i+index+1) >= Exp.Length() )  break;
          Char = Exp[i+index+1];
          index++;
        }
        if( index == ComparisonOperators.GetString(j).Length() )  {
          i += (index-1);
          coFactory = ComparisonOperators.GetObject(j);
          break;
        }
        Char = Exp.CharAt(i);           // roll back the character
      }
    }

    if( loFactory == NULL && (i < Exp.Length()) )  {
      for( size_t j=0; j < LogicalOperators.Count(); j++ )  {
        size_t index = 0;
        while( (index < LogicalOperators.GetString(j).Length()) &&
               (LogicalOperators.GetString(j).CharAt(index) == Char)  )
        {
          if( (i+index+1) >= Exp.Length() )  break;
          Char = Exp[i+index+1];
          index++;
        }
        if( index == LogicalOperators.GetString(j).Length() )  {
          i+= (index-1);
          loFactory = LogicalOperators.GetObject(j);
          break;
        }
        Char = Exp.CharAt(i);           // roll back the character
      }
    }

  }
  if( LogicalOperator != NULL )  return LogicalOperator;
  if( LeftCondition != NULL )  return LeftCondition;

  FErrors.Add( olxstr("Could not parse: ") << Exp);
  return NULL;
}
