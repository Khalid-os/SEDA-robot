t <!DOCTYPE html>
t     <html>
t       <head>
t         <meta content="text/html; charset=UTF-8" http-equiv="content-type">
t         <meta http-equiv="refresh" content="5; url=http://192.168.1.40/index.cgi">
t         <title>Control de LEDs</title>
t       </head>
t       <body> 
t         <form id="formulario" action="/index.cgi" method="get"> 
t           <table style="width: 200px" border="0">
t             <tbody>
t               <tr align="center">
t                 <td border="1" style="width: 100%">
t                   <div style="font-weight:bold">
t                     Practica 6
t                   </div>
t                 </td>
t               </tr>
t               <tr align="center">
t                 <td border="1" style="width: 100%">
t                   <div style="font-weight:bold">
t					  <label for ="velderecha">Velocidad derecha:</label>
t                     <input type="text" id="velderecha"  name="velderecha">
t                   </div>
t                 </td>
t               </tr>
t               <tr align="center">
t                 <td border="1" style="width: 100%">
t                   <div style="font-weight:bold">
t					  <label for ="velizq">Velocidad izquierda:</label>
t                     <input type="text" id="velizq"  name="velizq">
t                   </div>
t                 </td>
t               </tr>
t               <tr align="center">
t                 <td border="1" style="width: 100%">
t                   <div style="font-weight:bold">
t					  <label for ="umbral">Umbral:</label>
t                     <input type="text" id="umbral"  name="umbral">	
t                   </div>
t                 </td>
t               </tr>
t               <tr align="center">
t                 <td border="1" style="width: 100%">
t                   <div style="font-weight:bold">
t                       <input type="submit" value="Submit">
t                   </div>
t                 </td>
t               </tr>
t               <tr align="center">
t                 <td style="width: 100%">
c a 1               <input name="velderecha" value=%d onclick="submit()" type="text"> 
c a 2               <input name="velizq" value=%d onclick="submit()" type="text" >
t					<input name="umbral" value=%d onclick="submit()" type="text" ><br>
t                   <input name="Doomy" value="Doomy" type="hidden">
t                 </td>
t               </tr>
t             </tbody>
t           </table>
t           <table style="width: 200px" border="0">
t             <tbody>
t               <tr align="center">
tc            <td border="1" bgcolor="%s" style="width: 45%%">
t                   <div style="color:white;font-weight:bold">Led 1</div>
t                 </td>
t                 <td style="width: 10%">    <br>
t                 </td>
tc           <td border="1" bgcolor="%s"  style="width: 45%%">
t                   <div style="color:white;font-weight:bold">Led 2</div>
t                 </td>
t               </tr>
t             </tbody>
t           </table>
t         </form>
t         <br>
t       </body>
t     </html>
.