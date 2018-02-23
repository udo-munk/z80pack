// lpanel_data.h


/* Copyright (c) 2007-2008, John Kichury

   This software is freely distributable free of charge and without license fees with the 
   following conditions:

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   JOHN KICHURY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The above copyright notice must be included in any copies of this software.

*/

// graphics data for light panel


// 2d circle for lights

 static float cir2d_data[20][2] =
 {
  { 0., 1. },
  { -0.309017, 0.951057 },
  { -0.587785, 0.809017 },
  { -0.809017, 0.587785 },
  { -0.951057, 0.309017 },
  { -1., 0. },
  { -0.951057, -0.309017 },
  { -0.809017, -0.587785 },
  { -0.587785, -0.809017 },
  { -0.309017, -0.951057 },
  { 0., -1. } ,
  { 0.309017, -0.951057 },
  { 0.587785, -0.809017 },
  { 0.809017, -0.587785 },
  { 0.951057, -0.309017 },
  { 1., 0. },
  { 0.951057, 0.309017 },
  { 0.809017, 0.587785 },
  { 0.587785, 0.809017 },
  { 0.309017, 0.951057 } 
};

 static int cir2d_nverts = 20;

 static float cir2d_data2[20][2];

