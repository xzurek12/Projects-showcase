
//console.log("hello world");

function formsValid()
{
   var formsCollection = document.forms;
   
   // pro kazdy formular v dokumentu naleznu jeho inputy
   for (var i = 0; i < formsCollection.length; i++)
   {
      var formInputs = document.forms[i];
      var numberOfTextInputs = 0;
      var numberOfValidTextInputs = 0;
   
      // nalezeni poctu textovych inputu
      for (var j = 0; j < formInputs.length; j++)                 
      {
         if (formInputs.elements[j].type == "text") numberOfTextInputs += 1;    
      }   
      
      for (var j = 0; j < formInputs.length; j++)                 
      {
         if (formInputs.elements[j].type == "text") 
         {
            if ((getClass(formInputs.elements[j], "ok")) || (getClass(formInputs.elements[j], "blank"))) numberOfValidTextInputs += 1;
         }   
      }
      
      // nalezeni odesilacich tlacitek formulare
      for (var j = 0; j < formInputs.length; j++)                 
      {
         if (formInputs.elements[j].type == "submit")
         {
            if (numberOfTextInputs == numberOfValidTextInputs)
            {
                formInputs.elements[j].disabled = false;   
                formInputs.elements[j].value = "Odeslat"; 
            }
            else
            {
                formInputs.elements[j].disabled = true;   
                formInputs.elements[j].value = "Nelze odeslat"; 
            }
         }    
      }       
   }
}
//------------------------------------------------------------------------------
function getClass(inputElem, className)
{
   if (inputElem.className == className)
   {
      return true;
   }
   else
   {
      return false;
   }
}
//------------------------------------------------------------------------------
function setClass(inputElem, className)
{
   if (getClass(inputElem, className))
   {
      return;
   }
   else
   {
      inputElem.className = className;
   }
}
//------------------------------------------------------------------------------
function removeClass(inputElem)
{
   if (!getClass(inputElem, className))
   {
      return;
   }
   else
   {
      inputElem.className = "";
   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
function main(){

    formsValid();
  
    var formsCollection = document.forms;
    if (formsCollection.length <= 0) return false; // zadny formular v dokumentu
    
    // pro kazdy formular v dokumentu naleznu jeho inputy
    for (var i = 0; i < formsCollection.length; i++)
    {
        var formReady = false;  
        var formInputs = document.forms[i]; 

        if (formInputs.length <= 0) return false; // zadne inputy ve formulari
                                
        // loop through text inputs
        for (var j = 0; j < formInputs.length; j++)
        {        
           if (formInputs.elements[j].type == "text")
           {  
              if (formInputs.elements[j].required == false) setClass(formInputs.elements[j], "blank");
              
              // listener na zadani vstupu inputu formulare
              formInputs.elements[j].addEventListener("input", function(){ 
              
                  if (this.min == "") this.min = 0;
                  if (this.max == "") this.max = Number.MAX_SAFE_INTEGER; 
                  
                  if (this.pattern != "")
                  {
                      var re = RegExp(this.pattern);            
                      if (re.test(this.value))
                      {
                          setClass(this, "ok"); 
                          
                          if (this.value.length >= this.min) setClass(this, "ok");     
                          if (this.value.length < this.min)  setClass(this, "nok");       
                          if (this.value.length > this.max) setClass(this, "nok");   
                      } 
                      else
                      {
                          setClass(this, "nok");   
                      }
                  }
                  else
                  {                              
                      if (this.value.length >= this.min) setClass(this, "ok");   
                      if (this.value.length < this.min)  setClass(this, "nok");               
                      if (this.value.length > this.max) setClass(this, "nok"); 
                  }

                  if (this.value.length == 0) setClass(this, "blank"); 
                  if (this.required == true && this.value.length == 0) setClass(this, "nok");
                  
                  formsValid();

              }, false);  
           } 
        }   
    }
}
