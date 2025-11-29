#!/usr/bin/env php
<?php

if (!extension_loaded('bcg729')) {
    die("Extensão bcg729 não carregada!\n");
}

echo "=====================\n";
echo " TESTE DETALHADO DE MEMÓRIA\n";
echo "=====================\n\n";

function kb($b) {
    return sprintf("%.2f KB", $b / 1024);
}
function mb($b) {
    return sprintf("%.2f MB", $b / 1048576);
}

function memLine($label) {
    printf(
        "%-20s | uso atual: %-12s | real: %-12s | peak: %s\n",
        $label,
        kb(memory_get_usage()),
        kb(memory_get_usage(true)),
        kb(memory_get_peak_usage(true))
    );
}

function generateTestPCM() {
    $data = '';
    for ($i = 0; $i < 80; $i++) {
        $value = (int)(sin($i * 0.1) * 10000);
        $data .= pack('s', $value);
    }
    return $data;
}

function headerSection($title) {
    echo "\n--------------------------------------------\n";
    echo " $title\n";
    echo "--------------------------------------------\n";
}

$ITER = 1000;

headerSection("RESUMO INICIAL");
memLine("start");

/* -------------------------------------------------------------------------- */
/* TESTE 1: ENCODE/DECODE BC729                                               */
/* -------------------------------------------------------------------------- */

headerSection("TESTE 1 - bcg729 encode/decode ($ITER iterações)");

$start = memory_get_usage();
$startReal = memory_get_usage(true);
$peakBefore = memory_get_peak_usage(true);
var_dump(memory_get_usage());
$channel = new bcg729Channel();
$pcm = generateTestPCM();

echo "Rodando...\n";
for ($i = 0; $i < $ITER; $i++) {
    $e = $channel->encode($pcm);
    $d = $channel->decode($e);
    unset($e, $d);
}

$channel->close();
unset($channel, $pcm);
gc_collect_cycles();

$end = memory_get_usage();
$endReal = memory_get_usage(true);
$peakAfter = memory_get_peak_usage(true);

memLine("após teste 1");

printf("→ Diferença normal: %s\n", kb($end - $start));
printf("→ Diferença real:   %s\n", kb($endReal - $startReal));
printf("→ Pico aumentado:   %s\n", kb($peakAfter - $peakBefore));

/* -------------------------------------------------------------------------- */
/* TESTE 2: PCMA / PCMU                                                       */
/* -------------------------------------------------------------------------- */

headerSection("TESTE 2 - encode/decode PCMA/PCMU ($ITER iterações)");

$start = memory_get_usage();
$startReal = memory_get_usage(true);
$peakBefore = memory_get_peak_usage(true);

$pcm = generateTestPCM();

for ($i = 0; $i < $ITER; $i++) {
    $a = encodePcmToPcma($pcm);
    $b = decodePcmaToPcm($a);

    $c = encodePcmToPcmu($pcm);
    $d = decodePcmuToPcm($c);

    unset($a, $b, $c, $d);
}

unset($pcm);
gc_collect_cycles();

$end = memory_get_usage();
$endReal = memory_get_usage(true);
$peakAfter = memory_get_peak_usage(true);

memLine("após teste 2");

printf("→ Diferença normal: %s\n", kb($end - $start));
printf("→ Diferença real:   %s\n", kb($endReal - $startReal));
printf("→ Pico aumentado:   %s\n", kb($peakAfter - $peakBefore));

/* -------------------------------------------------------------------------- */
/* TESTE 3: MÚLTIPLAS INSTÂNCIAS                                              */
/* -------------------------------------------------------------------------- */

headerSection("TESTE 3 - múltiplas instâncias de bcg729Channel");

$start = memory_get_usage(true);
$peakBefore = memory_get_peak_usage(true);

for ($round = 0; $round < 100; $round++) {
    $chs = [];
    for ($i = 0; $i < 10; $i++) {
        $chs[] = new bcg729Channel();
    }
    foreach ($chs as $c) {
        $c->close();
    }
    unset($chs);
}

gc_collect_cycles();

$end = memory_get_usage(true);
$peakAfter = memory_get_peak_usage(true);

memLine("após teste 3");

printf("→ Diferença real:   %s\n", kb($end - $start));
printf("→ Pico aumentado:   %s\n", kb($peakAfter - $peakBefore));

/* -------------------------------------------------------------------------- */
/* TESTE 4: MIXAGEM                                                           */
/* -------------------------------------------------------------------------- */

headerSection("TESTE 4 - mixAudioChannels (".($ITER/10)." iterações)");

$start = memory_get_usage(true);
$peakBefore = memory_get_peak_usage(true);

$pcm = generateTestPCM();

for ($i = 0; $i < $ITER/10; $i++) {
    $m = mixAudioChannels([$pcm, $pcm, $pcm], 8000);
    unset($m);
}

unset($pcm);
gc_collect_cycles();

$end = memory_get_usage(true);
$peakAfter = memory_get_peak_usage(true);

memLine("após teste 4");

printf("→ Diferença real:   %s\n", kb($end - $start));
printf("→ Pico aumentado:   %s\n", kb($peakAfter - $peakBefore));

/* -------------------------------------------------------------------------- */
/* FINALIZAÇÃO                                                                */
/* -------------------------------------------------------------------------- */

headerSection("FINAL");

memLine("estado final");

$peak = memory_get_peak_usage(true);
printf("\nPico total absoluto: %s (%s)\n", kb($peak), mb($peak));

echo "\nTeste concluído.\n";
