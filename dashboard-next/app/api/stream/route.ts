import { getSnapshotStore } from "../../../server/snapshot-store";

export const dynamic = "force-dynamic";
export const runtime = "nodejs";

export async function GET(request: Request): Promise<Response> {
  const store = getSnapshotStore();
  const encoder = new TextEncoder();
  let tickTimer: ReturnType<typeof setInterval> | null = null;
  let pingTimer: ReturnType<typeof setInterval> | null = null;

  let cleanup = (): void => {};

  const stream = new ReadableStream<Uint8Array>({
    async start(controller) {
      let closed = false;
      let lastVersion = "";

      const stop = (): void => {
        if (closed) {
          return;
        }
        closed = true;
        request.signal.removeEventListener("abort", stop);
        if (tickTimer !== null) {
          clearInterval(tickTimer);
          tickTimer = null;
        }
        if (pingTimer !== null) {
          clearInterval(pingTimer);
          pingTimer = null;
        }
        try {
          controller.close();
        } catch {
          /* ja fechado */
        }
      };

      cleanup = stop;
      request.signal.addEventListener("abort", stop, { once: true });
      if (request.signal.aborted) {
        stop();
        return;
      }

      const send = (event: string, data: unknown): void => {
        if (closed) {
          return;
        }
        try {
          controller.enqueue(encoder.encode(`event: ${event}\ndata: ${JSON.stringify(data)}\n\n`));
        } catch {
          stop();
        }
      };

      const tick = async (): Promise<void> => {
        if (closed) {
          return;
        }
        try {
          const state = await store.get();
          if (state.kind === "ok" && state.snapshot !== null) {
            if (state.snapshot.version !== lastVersion) {
              lastVersion = state.snapshot.version;
              send("snapshot", {
                version: state.snapshot.version,
                observed_at: state.snapshot.observedAt
              });
            }
            return;
          }
          send("error", {
            ok: false,
            error: state.error,
            served_at: new Date().toISOString()
          });
        } catch {
          send("error", {
            ok: false,
            error: "snapshot_invalid",
            served_at: new Date().toISOString()
          });
        }
      };

      await tick();
      if (closed) return;
      tickTimer = setInterval(() => {
        void tick();
      }, 1000);
      pingTimer = setInterval(() => {
        if (!closed) {
          try {
            controller.enqueue(encoder.encode(": ping\n\n"));
          } catch {
            stop();
          }
        }
      }, 20_000);
    },
    cancel() {
      cleanup();
    }
  });

  return new Response(stream, {
    headers: {
      "content-type": "text/event-stream; charset=utf-8",
      "cache-control": "no-store, max-age=0",
      connection: "keep-alive",
      "x-accel-buffering": "no"
    }
  });
}
