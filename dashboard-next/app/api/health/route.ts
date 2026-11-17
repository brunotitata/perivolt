import { getSnapshotStore } from "../../../server/snapshot-store";
import { NO_STORE_HEADERS, healthHttp } from "../../../server/http";

export const dynamic = "force-dynamic";

export async function GET(): Promise<Response> {
  const result = await healthHttp(getSnapshotStore());
  return Response.json(result.body, { status: result.status, headers: NO_STORE_HEADERS });
}
