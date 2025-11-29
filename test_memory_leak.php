#!/usr/bin/env php
<?php
/**
 * Script de teste para detecção de vazamento de memória na extensão bcg729
 *
 * Este script testa todas as funções da extensão em loops intensivos
 * e monitora o uso de memória para detectar possíveis vazamentos.
 *
 * Uso: php test_memory_leak.php
 */

error_reporting(E_ALL);
ini_set('display_errors', 1);

// Cores para output no terminal
define('COLOR_RESET', "\033[0m");
define('COLOR_RED', "\033[31m");
define('COLOR_GREEN', "\033[32m");
define('COLOR_YELLOW', "\033[33m");
define('COLOR_BLUE', "\033[34m");
define('COLOR_MAGENTA', "\033[35m");
define('COLOR_CYAN', "\033[36m");

function formatBytes($bytes) {
    if ($bytes >= 1073741824) {
        return number_format($bytes / 1073741824, 2) . ' GB';
    } elseif ($bytes >= 1048576) {
        return number_format($bytes / 1048576, 2) . ' MB';
    } elseif ($bytes >= 1024) {
        return number_format($bytes / 1024, 2) . ' KB';
    }
    return $bytes . ' bytes';
}

function printHeader($title) {
    echo COLOR_CYAN . "\n" . str_repeat("=", 70) . "\n";
    echo "  " . $title . "\n";
    echo str_repeat("=", 70) . COLOR_RESET . "\n\n";
}

function printTest($name) {
    echo COLOR_BLUE . "[TEST] " . COLOR_RESET . $name . "... ";
}

function printSuccess($message = "OK") {
    echo COLOR_GREEN . "[✓] " . $message . COLOR_RESET . "\n";
}

function printError($message) {
    echo COLOR_RED . "[✗] " . $message . COLOR_RESET . "\n";
}

function printWarning($message) {
    echo COLOR_YELLOW . "[!] " . $message . COLOR_RESET . "\n";
}

function printInfo($message) {
    echo COLOR_MAGENTA . "[i] " . COLOR_RESET . $message . "\n";
}

// Verifica se a extensão está carregada
if (!extension_loaded('bcg729')) {
    printError("Extensão bcg729 não está carregada!");
    exit(1);
}

printHeader("TESTE DE VAZAMENTO DE MEMÓRIA - BCG729");

// Gera dados de teste
function generatePCMData($samples = 80) {
    $data = '';
    for ($i = 0; $i < $samples; $i++) {
        // Gera onda senoidal 16-bit little-endian
        $value = (int)(sin($i * 0.1) * 16000);
        $data .= pack('s', $value);
    }
    return $data;
}

function generateALawData($samples = 80) {
    $data = '';
    for ($i = 0; $i < $samples; $i++) {
        $data .= chr(rand(0, 255));
    }
    return $data;
}

function generateULawData($samples = 80) {
    return generateALawData($samples);
}

function generateG729Data($frames = 1) {
    $data = '';
    for ($i = 0; $i < $frames; $i++) {
        // G.729 tem 10 bytes por frame
        for ($j = 0; $j < 10; $j++) {
            $data .= chr(rand(0, 255));
        }
    }
    return $data;
}

// Configurações do teste
$iterations = 10000;  // Número de iterações
$check_interval = 1000; // Intervalo para verificar memória

printInfo("Configurações do teste:");
printInfo("  - Iterações: " . number_format($iterations));
printInfo("  - Intervalo de checagem: " . number_format($check_interval));
echo "\n";

// ============================================================================
// TESTE 1: bcg729Channel encode/decode
// ============================================================================
printHeader("TESTE 1: bcg729Channel encode/decode");

printTest("Testando encode/decode com " . number_format($iterations) . " iterações");
$memory_start = memory_get_usage(true);
$memory_peak_start = memory_get_peak_usage(true);
$memory_samples = [];

try {
    $channel = new bcg729Channel();

    for ($i = 0; $i < $iterations; $i++) {
        // Gera dados PCM (160 bytes = 80 samples de 16-bit)
        $pcm = generatePCMData(80);

        // Encode
        $encoded = $channel->encode($pcm);

        // Decode
        if ($encoded !== false) {
            $decoded = $channel->decode($encoded);
        }

        // Coleta amostra de memória
        if ($i % $check_interval == 0) {
            $current_memory = memory_get_usage(true);
            $memory_samples[] = [
                'iteration' => $i,
                'memory' => $current_memory,
                'delta' => $current_memory - $memory_start
            ];
        }

        // Força limpeza de variáveis
        unset($pcm, $encoded, $decoded);
    }

    $channel->close();
    unset($channel);

} catch (Exception $e) {
    printError("Erro: " . $e->getMessage());
}

// Força garbage collection
gc_collect_cycles();

$memory_end = memory_get_usage(true);
$memory_peak_end = memory_get_peak_usage(true);
$memory_diff = $memory_end - $memory_start;
$peak_diff = $memory_peak_end - $memory_peak_start;

echo "\n";
printInfo("Resultado:");
printInfo("  Memória inicial: " . formatBytes($memory_start));
printInfo("  Memória final: " . formatBytes($memory_end));
printInfo("  Diferença: " . formatBytes($memory_diff));
printInfo("  Pico de memória: " . formatBytes($peak_diff));

// Análise de vazamento
if ($memory_diff > 1048576) { // > 1MB
    printWarning("POSSÍVEL VAZAMENTO! Crescimento de memória: " . formatBytes($memory_diff));
} elseif ($memory_diff > 102400) { // > 100KB
    printWarning("Pequeno crescimento de memória detectado: " . formatBytes($memory_diff));
} else {
    printSuccess("Sem vazamento detectado");
}

// Mostra gráfico de crescimento
if (count($memory_samples) > 2) {
    echo "\n";
    printInfo("Crescimento de memória ao longo do tempo:");
    foreach ($memory_samples as $sample) {
        $bars = (int)($sample['delta'] / 10240); // 1 barra = 10KB
        printf("  Iter %5d: %s %s\n",
            $sample['iteration'],
            str_repeat('█', min($bars, 50)),
            formatBytes($sample['delta'])
        );
    }
}

// ============================================================================
// TESTE 2: Funções de conversão PCMA/PCMU
// ============================================================================
printHeader("TESTE 2: Conversões PCMA/PCMU");

$tests = [
    'decodePcmaToPcm' => ['input' => generateALawData(160), 'name' => 'A-law decode'],
    'decodePcmuToPcm' => ['input' => generateULawData(160), 'name' => 'μ-law decode'],
    'encodePcmToPcma' => ['input' => generatePCMData(80), 'name' => 'A-law encode'],
    'encodePcmToPcmu' => ['input' => generatePCMData(80), 'name' => 'μ-law encode'],
];

foreach ($tests as $function => $test) {
    printTest("Testando {$test['name']}");

    $memory_start = memory_get_usage(true);

    for ($i = 0; $i < $iterations; $i++) {
        $result = $function($test['input']);
        unset($result);
    }

    gc_collect_cycles();

    $memory_end = memory_get_usage(true);
    $memory_diff = $memory_end - $memory_start;

    if ($memory_diff > 102400) { // > 100KB
        printWarning("Crescimento: " . formatBytes($memory_diff));
    } else {
        printSuccess("OK (" . formatBytes($memory_diff) . ")");
    }
}

// ============================================================================
// TESTE 3: Conversões L16 (endianness)
// ============================================================================
printHeader("TESTE 3: Conversões L16");

$tests = [
    'decodeL16ToPcm' => ['input' => generatePCMData(80), 'name' => 'L16 decode'],
    'encodePcmToL16' => ['input' => generatePCMData(80), 'name' => 'L16 encode'],
    'pcmLeToBe' => ['input' => generatePCMData(80), 'name' => 'LE to BE'],
];

foreach ($tests as $function => $test) {
    printTest("Testando {$test['name']}");

    $memory_start = memory_get_usage(true);

    for ($i = 0; $i < $iterations; $i++) {
        $result = $function($test['input']);
        unset($result);
    }

    gc_collect_cycles();

    $memory_end = memory_get_usage(true);
    $memory_diff = $memory_end - $memory_start;

    if ($memory_diff > 102400) {
        printWarning("Crescimento: " . formatBytes($memory_diff));
    } else {
        printSuccess("OK (" . formatBytes($memory_diff) . ")");
    }
}

// ============================================================================
// TESTE 4: Mixagem de canais
// ============================================================================
printHeader("TESTE 4: Mixagem de múltiplos canais");

printTest("Testando mixAudioChannels com 4 canais");

$memory_start = memory_get_usage(true);

for ($i = 0; $i < $iterations / 10; $i++) { // Menos iterações, mais pesado
    $channels = [
        generatePCMData(160),
        generatePCMData(160),
        generatePCMData(160),
        generatePCMData(160),
    ];

    $mixed = mixAudioChannels($channels, 8000);
    unset($channels, $mixed);
}

gc_collect_cycles();

$memory_end = memory_get_usage(true);
$memory_diff = $memory_end - $memory_start;

if ($memory_diff > 524288) { // > 512KB
    printWarning("Crescimento: " . formatBytes($memory_diff));
} else {
    printSuccess("OK (" . formatBytes($memory_diff) . ")");
}

// ============================================================================
// TESTE 5: Teste de stress com múltiplas instâncias
// ============================================================================
printHeader("TESTE 5: Múltiplas instâncias simultâneas");

printTest("Criando e destruindo múltiplas instâncias");

$memory_start = memory_get_usage(true);
$channels_count = 100;

for ($round = 0; $round < 100; $round++) {
    $channels = [];

    // Cria múltiplas instâncias
    for ($i = 0; $i < $channels_count; $i++) {
        $channels[] = new bcg729Channel();
    }

    // Usa as instâncias
    $pcm = generatePCMData(80);
    foreach ($channels as $ch) {
        $encoded = $ch->encode($pcm);
        unset($encoded);
    }

    // Fecha e libera
    foreach ($channels as $ch) {
        $ch->close();
    }
    unset($channels);

    gc_collect_cycles();
}

$memory_end = memory_get_usage(true);
$memory_diff = $memory_end - $memory_start;

if ($memory_diff > 1048576) {
    printWarning("Crescimento: " . formatBytes($memory_diff));
} else {
    printSuccess("OK (" . formatBytes($memory_diff) . ")");
}

// ============================================================================
// TESTE 6: Teste de vazamento com dados grandes
// ============================================================================
printHeader("TESTE 6: Processamento de grandes volumes");

printTest("Processando arquivos grandes simulados");

$memory_start = memory_get_usage(true);

for ($i = 0; $i < 100; $i++) {
    $channel = new bcg729Channel();

    // Simula 10 segundos de áudio (8000 samples/sec, 80 samples/frame)
    $frames = 1000;
    $pcm_data = generatePCMData(80 * $frames);

    // Processa em chunks
    for ($frame = 0; $frame < $frames; $frame++) {
        $chunk = substr($pcm_data, $frame * 160, 160);
        $encoded = $channel->encode($chunk);
        if ($encoded !== false) {
            $decoded = $channel->decode($encoded);
        }
        unset($chunk, $encoded, $decoded);
    }

    $channel->close();
    unset($channel, $pcm_data);

    if ($i % 10 == 0) {
        gc_collect_cycles();
    }
}

gc_collect_cycles();

$memory_end = memory_get_usage(true);
$memory_diff = $memory_end - $memory_start;

if ($memory_diff > 2097152) { // > 2MB
    printWarning("Crescimento: " . formatBytes($memory_diff));
} else {
    printSuccess("OK (" . formatBytes($memory_diff) . ")");
}

// ============================================================================
// RESUMO FINAL
// ============================================================================
printHeader("RESUMO FINAL");

$total_memory = memory_get_peak_usage(true);
printInfo("Pico total de memória usado: " . formatBytes($total_memory));

echo "\n";
printInfo("Para análise mais detalhada, use:");
printInfo("  valgrind --leak-check=full --show-leak-kinds=all php test_memory_leak.php");
printInfo("  ou");
printInfo("  USE_ZEND_ALLOC=0 valgrind --leak-check=full php test_memory_leak.php");

echo "\n";
printSuccess("Testes concluídos!");
