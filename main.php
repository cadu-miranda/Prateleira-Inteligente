<?php
$AB1 = $_POST["alimentoB1"]; // armazenamento da string de saída do arduino
$file = 'index.html'; // redirecionamento dos dados ao site principal
$lista = list($A1, $B1, $A2, $B2)= explode("=", $AB1); /* desconstrução da string de saída do arduino (A1 = Alimento da Balança 1; B1 = Peso Atual da Balança 1; A2 = Alimento da Balança 2; B2 = Peso Atual da Balança 2) */
$data ="
<!DOCTYPE html>
<html lang='pt-br'>

<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <meta http-equiv='refresh' content='2'>
    <title>Projeto de TCC</title>
    <style> 
        table,
        th,
        td {
            border: 1px solid black;
            border-collapse: collapse;
            margin-left: auto;
            margin-right: auto;
            margin-top: 22vh;
        }

        th,
        td {
            padding: 15px;
            text-align: center;
        }
    </style>
</head>

<body>
    <h2 style='font-family: Verdana, Geneva, Tahoma, sans-serif; color: blueviolet; text-align: center;'>Protótipo
        de prateleira inteligente com Arduino</h2>
    <p
        style='font-family: Verdana, Geneva, Tahoma, sans-serif; color: blueviolet; text-align: center; font-size: 14pt;'>
        Integrantes:
        Carlos E. Miranda e Thiago T. Yara</p>
    <table style=width:55%;> <!-- início da criação da tabela -->
        <tr>
            <th>Alimentos</th>
            <th>Pesos</th>
        </tr>
        <tr>
            <td>$lista[0]</td> <!-- armazena na 1ª coluna, 2ª linha da tabela o alimento da balança 1 (A1) -->
            <td>$lista[1] kg</td> <!-- armazena na 1ª coluna, 3ª linha da tabela o peso atual da balança 1 (B1) -->
        </tr>
        <tr>
            <td>$lista[2]</td> <!-- armazena na 2ª coluna, 2ª linha da tabela o alimento da balança 2 (A2) -->
            <td>$lista[3] kg</td> <!-- armazena na 2ª coluna, 3ª linha da tabela o peso atual da balança 2 (B2) -->
        </tr>
    </table> <!-- fim da criação da tabela -->

</body>

</html>
"; // formatação do conteúdo da string de saída
file_put_contents($file, $data); // envio dos dados já formatados ao arquivo index.html
echo 'Redirecionamento...'
?>
