t <!DOCTYPE html>
t    <html>
t       	<head>
t         	<meta content="text/html; charset=UTF-8" http-equiv="content-type">
t        	<meta http-equiv="refresh" content="20; url=http://192.168.1.120/mindex.cgi">
t         	<title>Control vbles</title>
t         	<style type="text/css">
t         		.tabla_1
t {
t width:40%; 
t text-align:center;
t }
t .tabla_2
t {
t width:20%; 
t text-align:center;
t }
t         	</style>
t        </head>
t        <body>
t           <table border="1">
t              <tbody>
t                   <tr>
t                       <td class="tabla_1"> Variables: </td>
t                       <td class="tabla_2"> Valores: </td>
t                       <td align="center" width="100"> Gráfico </td>
t                   </tr>
t                   <tr>
t                       <td class="tabla_1"> Distancia </td>
c 1                     <td class="tabla_2"> %4.1f </td>
c 2                     <td align="center" width="%f" bgcolor="blue"></td>
t                   </tr>
t                   <tr>
t                       <td class="tabla_1"> difSensores </td>
c 3                     <td class="tabla_2"> %3f </td>
c 4                     <td align="center" width="%f" bgcolor="blue"></td>
t                   </tr>
t                   <tr>
t   					<td class="tabla_1"> mediaSensores </td>
c 5						<td class="tabla_2"> %3f </td>
c 6						<td align="center" width="%f" bgcolor="blue"></td>
t                   </tr>
t                   <tr>
t                       <td class="tabla_1"> umbral </td>
c 7                     <td class="tabla_2"> %f </td>
c 8						<td align="center" width="%f" bgcolor="blue"></td>
t   				</tr>
t   			</tbody>
t    		</table>
t   		<form id="formulario" action="/mindex.cgi" method="get"> 
t           	<table style="width: 200px" border="0">
t           	<tbody>
t            		<tr align="center">
t                 		<td border="1" style="width: 100%">
t                  			<div style="font-weight:bold">
t                     			Cambiar valor de umbral:
t                   			</div>
t                		</td>
t              		</tr>
t               		<tr align="center">
t               			<td border="1" style="width: 100%">
t                   			<div style="font-weight:bold">
t                    		 Nuevo valor:<input name="valor" value=""  type="number">
t                   			</div>
t                 		</td>
t                 		<td border="1" style="width: 100%">
t                   			<div style="font-weight:bold"><br>
t                    		 <button type="submit">Actualizar</button>
t                   			</div>
t                 		</td>
t                 		</tr>
t           		</tbody>
t       </table>
t   </form>
t   	</body>
.