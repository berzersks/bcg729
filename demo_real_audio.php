<?php
/**
 * Demonstração com Áudio Real (mixed.pcm)
 *
 * Este exemplo usa o arquivo mixed.pcm para demonstrar todas as funcionalidades
 * implementadas de forma prática e realista.
 */

echo "=== DEMONSTRAÇÃO COM ÁUDIO REAL ===\n\n";

// Função para criar arquivo WAV a partir de PCM
function createWavFile($pcm_data, $sample_rate, $output_file) {
    $num_samples = strlen($pcm_data) / 2;
    $num_channels = 1; // Mono
    $bits_per_sample = 16;
    $byte_rate = $sample_rate * $num_channels * ($bits_per_sample / 8);
    $block_align = $num_channels * ($bits_per_sample / 8);
    $data_size = strlen($pcm_data);

    // Header WAV
    $wav = '';
    $wav .= 'RIFF';
    $wav .= pack('V', $data_size + 36); // Tamanho do arquivo - 8
    $wav .= 'WAVE';
    $wav .= 'fmt ';
    $wav .= pack('V', 16); // Tamanho do chunk fmt
    $wav .= pack('v', 1);  // Formato PCM
    $wav .= pack('v', $num_channels);
    $wav .= pack('V', $sample_rate);
    $wav .= pack('V', $byte_rate);
    $wav .= pack('v', $block_align);
    $wav .= pack('v', $bits_per_sample);
    $wav .= 'data';
    $wav .= pack('V', $data_size);
    $wav .= $pcm_data;

    file_put_contents($output_file, $wav);
    return strlen($wav);
}

// ====== 0. CARREGA ARQUIVO PCM REAL ======
$pcm_file = __DIR__ . '/mixed.pcm';
if (!file_exists($pcm_file)) {
    die("❌ Erro: arquivo mixed.pcm não encontrado!\n");
}

$pcm_original = file_get_contents($pcm_file);
$sample_rate = 8000; // Taxa típica para telefonia
$duration = strlen($pcm_original) / 2 / $sample_rate;

echo "📂 Arquivo PCM carregado: mixed.pcm\n";
echo "   Tamanho: " . number_format(strlen($pcm_original)) . " bytes (" .
     round(strlen($pcm_original) / 1024, 1) . " KB)\n";
echo "   Amostras: " . number_format(strlen($pcm_original) / 2) . "\n";
echo "   Duração: ~" . round($duration, 2) . " segundos @ {$sample_rate} Hz\n";
echo "   Formato: PCM 16-bit signed little-endian\n\n";

// Converte o arquivo original para WAV
$wav_size = createWavFile($pcm_original, $sample_rate, __DIR__ . '/0_original.wav');
echo "🎵 Convertido para: 0_original.wav (" . round($wav_size / 1024, 1) . " KB)\n\n";

// ====== 1. ENCODE PARA PCMA (A-law) ======
echo "1️⃣  Encode PCM → PCMA (A-law)\n";
echo str_repeat("─", 60) . "\n";

$start = microtime(true);
$pcma_encoded = encodePcmToPcma($pcm_original);
$time_encode_pcma = microtime(true) - $start;

$compression_pcma = (1 - strlen($pcma_encoded) / strlen($pcm_original)) * 100;

echo "✓ Encoded em " . round($time_encode_pcma * 1000, 2) . " ms\n";
echo "  Tamanho original: " . number_format(strlen($pcm_original)) . " bytes\n";
echo "  Tamanho PCMA: " . number_format(strlen($pcma_encoded)) . " bytes\n";
echo "  Compressão: " . round($compression_pcma, 1) . "%\n";
echo "  Taxa: " . round(strlen($pcm_original) / 1024 / $time_encode_pcma, 1) . " KB/s\n\n";

// Salva PCMA
file_put_contents(__DIR__ . '/mixed.pcma', $pcma_encoded);
echo "💾 Salvo: mixed.pcma\n\n";

// ====== 2. DECODE PCMA → PCM ======
echo "2️⃣  Decode PCMA → PCM\n";
echo str_repeat("─", 60) . "\n";

$start = microtime(true);
$pcm_from_pcma = decodePcmaToPcm($pcma_encoded);
$time_decode_pcma = microtime(true) - $start;

echo "✓ Decoded em " . round($time_decode_pcma * 1000, 2) . " ms\n";
echo "  Tamanho recuperado: " . number_format(strlen($pcm_from_pcma)) . " bytes\n";
echo "  Validação: " . (strlen($pcm_from_pcma) === strlen($pcm_original) ? "✓ OK" : "✗ ERRO") . "\n";
echo "  Taxa: " . round(strlen($pcm_from_pcma) / 1024 / $time_decode_pcma, 1) . " KB/s\n\n";

// Salva PCM recuperado e converte para WAV
file_put_contents(__DIR__ . '/mixed_from_pcma.pcm', $pcm_from_pcma);
$wav_size = createWavFile($pcm_from_pcma, $sample_rate, __DIR__ . '/1_from_pcma.wav');
echo "💾 Salvo: mixed_from_pcma.pcm\n";
echo "🎵 Convertido para: 1_from_pcma.wav (" . round($wav_size / 1024, 1) . " KB)\n\n";

// ====== 3. ENCODE PARA PCMU (μ-law) ======
echo "3️⃣  Encode PCM → PCMU (μ-law)\n";
echo str_repeat("─", 60) . "\n";

$start = microtime(true);
$pcmu_encoded = encodePcmToPcmu($pcm_original);
$time_encode_pcmu = microtime(true) - $start;

$compression_pcmu = (1 - strlen($pcmu_encoded) / strlen($pcm_original)) * 100;

echo "✓ Encoded em " . round($time_encode_pcmu * 1000, 2) . " ms\n";
echo "  Tamanho PCMU: " . number_format(strlen($pcmu_encoded)) . " bytes\n";
echo "  Compressão: " . round($compression_pcmu, 1) . "%\n";
echo "  Taxa: " . round(strlen($pcm_original) / 1024 / $time_encode_pcmu, 1) . " KB/s\n\n";

// Salva PCMU
file_put_contents(__DIR__ . '/mixed.pcmu', $pcmu_encoded);
echo "💾 Salvo: mixed.pcmu\n\n";

// ====== 4. DECODE PCMU → PCM ======
echo "4️⃣  Decode PCMU → PCM\n";
echo str_repeat("─", 60) . "\n";

$start = microtime(true);
$pcm_from_pcmu = decodePcmuToPcm($pcmu_encoded);
$time_decode_pcmu = microtime(true) - $start;

echo "✓ Decoded em " . round($time_decode_pcmu * 1000, 2) . " ms\n";
echo "  Tamanho recuperado: " . number_format(strlen($pcm_from_pcmu)) . " bytes\n";
echo "  Validação: " . (strlen($pcm_from_pcmu) === strlen($pcm_original) ? "✓ OK" : "✗ ERRO") . "\n\n";

file_put_contents(__DIR__ . '/mixed_from_pcmu.pcm', $pcm_from_pcmu);
$wav_size = createWavFile($pcm_from_pcmu, $sample_rate, __DIR__ . '/2_from_pcmu.wav');
echo "💾 Salvo: mixed_from_pcmu.pcm\n";
echo "🎵 Convertido para: 2_from_pcmu.wav (" . round($wav_size / 1024, 1) . " KB)\n\n";

// ====== 5. CONVERSÃO L16 (Big-Endian) ======
echo "5️⃣  Conversão PCM ↔ L16 (Endianness)\n";
echo str_repeat("─", 60) . "\n";

$start = microtime(true);
$l16 = encodePcmToL16($pcm_original);
$time_l16 = microtime(true) - $start;

echo "✓ PCM → L16 em " . round($time_l16 * 1000, 2) . " ms\n";
echo "  Tamanho L16: " . number_format(strlen($l16)) . " bytes\n";

$pcm_from_l16 = decodeL16ToPcm($l16);
$reversible = ($pcm_original === $pcm_from_l16);

echo "✓ L16 → PCM reversível: " . ($reversible ? "✓ SIM" : "✗ NÃO") . "\n\n";

file_put_contents(__DIR__ . '/mixed.l16', $l16);
echo "💾 Salvo: mixed.l16\n\n";

// ====== 6. MIXAGEM: DIVIDIR E MIXAR ======
echo "6️⃣  Mixagem de Canais\n";
echo str_repeat("─", 60) . "\n";

// Divide o áudio em 3 partes para simular 3 canais
$total_samples = strlen($pcm_original) / 2;
$part_size = (int)($total_samples / 3) * 2; // em bytes

$channel1 = substr($pcm_original, 0, $part_size);
$channel2 = substr($pcm_original, $part_size, $part_size);
$channel3 = substr($pcm_original, $part_size * 2);

echo "Canal 1: " . number_format(strlen($channel1)) . " bytes\n";
echo "Canal 2: " . number_format(strlen($channel2)) . " bytes\n";
echo "Canal 3: " . number_format(strlen($channel3)) . " bytes\n\n";

$start = microtime(true);
$mixed = mixAudioChannels([$channel1, $channel2, $channel3], $sample_rate);
$time_mix = microtime(true) - $start;

echo "✓ Mixado em " . round($time_mix * 1000, 2) . " ms\n";
echo "  Resultado: " . number_format(strlen($mixed)) . " bytes\n";
echo "  Duração: ~" . round(strlen($mixed) / 2 / $sample_rate, 2) . " segundos\n";
echo "  Normalização: √3 aplicada\n\n";

file_put_contents(__DIR__ . '/mixed_3channels.pcm', $mixed);
$wav_size = createWavFile($mixed, $sample_rate, __DIR__ . '/3_mixed_3channels.wav');
echo "💾 Salvo: mixed_3channels.pcm\n";
echo "🎵 Convertido para: 3_mixed_3channels.wav (" . round($wav_size / 1024, 1) . " KB)\n\n";

// ====== 7. PIPELINE COMPLETO: PCM → PCMA → PCM → MIX ======
echo "7️⃣  Pipeline Completo\n";
echo str_repeat("─", 60) . "\n";
echo "Fluxo: PCM → PCMA → PCM → Mix de 2 canais\n\n";

// Pega primeira e segunda metade do áudio
$half = (int)(strlen($pcm_original) / 4) * 2;
$ch1 = substr($pcm_original, 0, $half);
$ch2 = substr($pcm_original, $half, $half);

echo "Passo 1: Encode canais para PCMA\n";
$ch1_pcma = encodePcmToPcma($ch1);
$ch2_pcma = encodePcmToPcma($ch2);
echo "  Canal 1: " . strlen($ch1) . "B → " . strlen($ch1_pcma) . "B\n";
echo "  Canal 2: " . strlen($ch2) . "B → " . strlen($ch2_pcma) . "B\n\n";

echo "Passo 2: Decode PCMA para PCM\n";
$ch1_decoded = decodePcmaToPcm($ch1_pcma);
$ch2_decoded = decodePcmaToPcm($ch2_pcma);
echo "  Canal 1: " . strlen($ch1_decoded) . "B recuperados\n";
echo "  Canal 2: " . strlen($ch2_decoded) . "B recuperados\n\n";

echo "Passo 3: Mix dos canais\n";
$final_mixed = mixAudioChannels([$ch1_decoded, $ch2_decoded], $sample_rate);
echo "  Resultado: " . strlen($final_mixed) . "B\n\n";

file_put_contents(__DIR__ . '/pipeline_result.pcm', $final_mixed);
$wav_size = createWavFile($final_mixed, $sample_rate, __DIR__ . '/4_pipeline_result.wav');


$wav_size = createWavFile(resampler($final_mixed, 8000, 44100), 44100, __DIR__ . '/44100.wav');


echo "💾 Salvo: pipeline_result.pcm\n";
echo "🎵 Convertido para: 4_pipeline_result.wav (" . round($wav_size / 1024, 1) . " KB)\n\n";

// ====== 8. ANÁLISE DE QUALIDADE ======
echo "8️⃣  Análise de Qualidade (SNR)\n";
echo str_repeat("─", 60) . "\n";

function calculateSNR($original, $decoded) {
    $limit = min(strlen($original), strlen($decoded), 20000); // Limita para performance

    $orig_samples = unpack('s*', substr($original, 0, $limit));
    $dec_samples = unpack('s*', substr($decoded, 0, $limit));

    $signal_power = 0;
    $noise_power = 0;

    foreach ($orig_samples as $i => $orig) {
        $signal_power += $orig * $orig;
        $diff = $orig - $dec_samples[$i];
        $noise_power += $diff * $diff;
    }

    if ($noise_power == 0) return INF;
    return 10 * log10($signal_power / $noise_power);
}

$snr_pcma = calculateSNR($pcm_original, $pcm_from_pcma);
$snr_pcmu = calculateSNR($pcm_original, $pcm_from_pcmu);

echo "Signal-to-Noise Ratio (SNR):\n";
echo "  PCMA (A-law): " . round($snr_pcma, 2) . " dB\n";
echo "  PCMU (μ-law): " . round($snr_pcmu, 2) . " dB\n\n";

// Interpreta os resultados
if ($snr_pcma > 35) {
    echo "  📊 PCMA: Excelente qualidade para voz\n";
} elseif ($snr_pcma > 30) {
    echo "  📊 PCMA: Boa qualidade para voz\n";
} else {
    echo "  📊 PCMA: Qualidade aceitável\n";
}

if ($snr_pcmu > 35) {
    echo "  📊 PCMU: Excelente qualidade para voz\n";
} elseif ($snr_pcmu > 30) {
    echo "  📊 PCMU: Boa qualidade para voz\n";
} else {
    echo "  📊 PCMU: Qualidade aceitável\n";
}

echo "\n";

// ====== 9. RESUMO FINAL ======
echo "═══════════════════════════════════════════════════════════\n";
echo "✅ RESUMO DA DEMONSTRAÇÃO\n";
echo "═══════════════════════════════════════════════════════════\n\n";

echo "📁 Arquivos gerados:\n\n";
echo "🎵 Arquivos WAV (prontos para ouvir):\n";
echo "   • 0_original.wav - Áudio original\n";
echo "   • 1_from_pcma.wav - Decodificado de A-law\n";
echo "   • 2_from_pcmu.wav - Decodificado de μ-law\n";
echo "   • 3_mixed_3channels.wav - Mix de 3 canais\n";
echo "   • 4_pipeline_result.wav - Pipeline completo\n\n";
echo "📦 Arquivos intermediários:\n";
echo "   • mixed.pcma - Codificado em A-law (comprimido)\n";
echo "   • mixed.pcmu - Codificado em μ-law (comprimido)\n";
echo "   • mixed.l16 - Formato L16 (big-endian)\n";
echo "   • *.pcm - Arquivos PCM brutos\n\n";

echo "⚡ Performance:\n";
echo "   • Encode PCMA: " . round($time_encode_pcma * 1000, 2) . " ms\n";
echo "   • Decode PCMA: " . round($time_decode_pcma * 1000, 2) . " ms\n";
echo "   • Encode PCMU: " . round($time_encode_pcmu * 1000, 2) . " ms\n";
echo "   • Decode PCMU: " . round($time_decode_pcmu * 1000, 2) . " ms\n";
echo "   • Mixagem: " . round($time_mix * 1000, 2) . " ms\n\n";

echo "💾 Economia de espaço:\n";
echo "   • PCM original: " . round(strlen($pcm_original) / 1024, 1) . " KB\n";
echo "   • PCMA: " . round(strlen($pcma_encoded) / 1024, 1) . " KB (50% menor)\n";
echo "   • PCMU: " . round(strlen($pcmu_encoded) / 1024, 1) . " KB (50% menor)\n\n";

echo "🎯 Funções testadas:\n";
echo "   ✓ encodePcmToPcma() - Encode para A-law\n";
echo "   ✓ decodePcmaToPcm() - Decode de A-law\n";
echo "   ✓ encodePcmToPcmu() - Encode para μ-law\n";
echo "   ✓ decodePcmuToPcm() - Decode de μ-law\n";
echo "   ✓ encodePcmToL16() - PCM LE → L16 BE\n";
echo "   ✓ decodeL16ToPcm() - L16 BE → PCM LE\n";
echo "   ✓ mixAudioChannels() - Mix com normalização\n\n";

echo "🎵 Características da mixagem:\n";
echo "   • Normalização automática: 1/√N\n";
echo "   • Soft clipping para evitar distorção\n";
echo "   • Buffer 32-bit anti-overflow\n";
echo "   • Suporte para canais de tamanhos diferentes\n\n";

echo "═══════════════════════════════════════════════════════════\n";
echo "✨ DEMONSTRAÇÃO CONCLUÍDA COM SUCESSO! ✨\n";
echo "═══════════════════════════════════════════════════════════\n\n";

echo "🔊 Como ouvir os arquivos:\n\n";
echo "Linux:\n";
echo "   aplay 0_original.wav\n";
echo "   aplay 1_from_pcma.wav\n";
echo "   aplay 2_from_pcmu.wav\n";
echo "   aplay 3_mixed_3channels.wav\n";
echo "   aplay 4_pipeline_result.wav\n\n";

echo "Windows:\n";
echo "   start 0_original.wav\n\n";

echo "macOS:\n";
echo "   afplay 0_original.wav\n\n";

echo "Ou simplesmente clique duplo nos arquivos .wav!\n\n";

echo "💡 Dica: Compare a qualidade entre:\n";
echo "   • 0_original.wav (original)\n";
echo "   • 1_from_pcma.wav (passou por compressão A-law)\n";
echo "   • 2_from_pcmu.wav (passou por compressão μ-law)\n";
echo "   • 3_mixed_3channels.wav (mixagem de 3 partes)\n";
echo "   • 4_pipeline_result.wav (pipeline completo)\n";
