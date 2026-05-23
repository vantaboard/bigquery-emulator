# Use BigQuery agent analytics

BigQuery agent analytics is an open source solution that lets you
capture, analyze, and visualize multimodal agent interaction data at scale.
BigQuery agent analytics lets you stream raw agent interactions
(requests, responses, tool calls, and errors) directly into
BigQuery. Doing so lets you
perform AI-powered evaluation, optimize agent prompts, and extract long-term
memory to enhance future interactions.

BigQuery agent analytics is supported in [Agent Development Kit (ADK)](https://google.github.io/adk-docs/integrations/bigquery-agent-analytics/)
and
[LangGraph](https://docs.langchain.com/oss/python/integrations/callbacks/google_bigquery)
(Preview).

## Architecture

BigQuery agent analytics facilitates streaming of agent activity
data to BigQuery. By using the BigQuery Storage Write API,
this solution provides high-throughput, low-latency log streaming without
blocking agent execution.

The data flow includes the following stages:

1. **Capture**. Plugins in the Agent Development Kit (ADK) or callbacks in LangGraph intercept interaction events.
2. **Stream**. Interaction events are sent to BigQuery through the Storage Write API. If a standardized schema doesn't exist, the agent creates one automatically.
3. **Consume** . Derive insights by using prebuilt dashboards, a data agent, or SQL and advanced BigQuery ML features. To improve debugging and evaluation, you can use a Python SDK. For advanced debugging and use cases for agent evaluation, you can also use the [BigQuery
   agent analytics SDK](https://google.github.io/adk-docs/integrations/bigquery-agent-analytics/).

![The BigQuery agent analytics data flow from agent orchestration frameworks to BigQuery](https://docs.cloud.google.com/static/bigquery/images/agent-analytics-architecture.png)

### Agent analytics benefits

- Enable comprehensive logging with a single line of code and automate schema management.
- Log and analyze multimodal data including text, images, video, and audio using object tables.
- Track operational metrics like token consumption and latency within a robust, predefined schema.
- Identify optimization opportunities using BigQuery generative AI functions and vector search.
- Secure agent logs with granular access controls, data masking, and encryption.

## Examples of working with agent log data

The following are some common use cases and examples for working with agent
log data.

### Observability and operational metrics

- Load the [pre-built dashboard](https://google.github.io/adk-docs/integrations/bigquery-agent-analytics#looker-studio-dashboard) and configure the report with your table to find agents with high token consumption, errors, or long session lengths.
- [Use SQL](https://google.github.io/adk-docs/integrations/bigquery-agent-analytics#automatically-created-views-1270) to break down costs by agent flows and determine if a specific agent, such as a refinement agent, consumes a disproportionate amount of tokens compared to its contribution to final responses.
- [Use the BigQuery conversational analytics agent](https://docs.cloud.google.com/bigquery/docs/conversational-analytics) for AI-powered root cause analysis by running queries with [the `AI.GENERATE`
  function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-generate). For example, "Analyze this conversation log and explain the root cause of the failure."

### Agent evaluation and quality analysis

- Use the [`AI.SCORE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score) to rank conversations and measure the agent's rank over time.
- Use a SQL query with [Vector Search](https://docs.cloud.google.com/bigquery/docs/vector-search-intro) to identify conversation clusters where the agent failed to assist users, and then compare them to the user's original intent. Doing so helps to find gaps in the agent's toolset or knowledge base.

### Business insights and contextualization

- Perform a JOIN between the `agent_events` table and other business tables to contextualize agent data. For example, show the average order value (AOV) for customers who interacted with the AI agent as opposed to customers who used the search bar.

For more examples, see [Advanced analysis queries](https://google.github.io/adk-docs/integrations/bigquery-agent-analytics/#advanced-analysis-queries).

## Work with BigQuery agent analytics

To integrate BigQuery agent analytics into your workflow, see the
documentation for your framework:

- [ADK BigQuery Analytics plugin guide](https://google.github.io/adk-docs/integrations/bigquery-agent-analytics/)
- [BigQuery callback handler
  integration](https://docs.langchain.com/oss/python/integrations/callbacks/google_bigquery) (LangChain and LangGraph)

## What's next

- [Explore the codelab](https://codelabs.developers.google.com/adk-bigquery-agent-analytics-plugin#0).